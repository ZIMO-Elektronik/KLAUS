#include "include/process/mdu_ein/update.hpp"

#include <iostream>
#include <thread>

namespace process::mdu_ein {

Update::Update(std::filesystem::path path,
               type::MDUEntryType entry_type,
               std::vector<uint32_t> decoder_ids)
  : Base{}, _zsu{std::make_shared<libklug::ZSU>(path)},
    _decoderIDs{decoder_ids}, _entryType{entry_type} {
  _lib.registerCb<[] {}>(
    [this](res::Result const result) { this->handle_result(result); });
}

Update::Update(std::shared_ptr<libklug::ZSU> zsu,
               type::MDUEntryType entry_type,
               std::vector<uint32_t> decoder_ids)
  : Base{}, _zsu{zsu}, _decoderIDs{decoder_ids}, _entryType{entry_type} {
  assert(_zsu != nullptr);
  assert(_zsu->valid());

  _lib.registerCb<[] {}>(
    [this](res::Result const result) { this->handle_result(result); });
}

Update::~Update() {
  _lib.deregisterCb();
  disconnect();
}

bool Update::execute() {
  if (_zsu == nullptr || !_zsu->valid() || !connect()) {
    _done = true;
    return false;
  }
  modeAction();
  return true;
}

void Update::abort() { _abort = true; }

void Update::handle_result(res::Result r) {
  if (_abort) resetAction();
  std::invoke(_state, this, r);
}

void Update::modeAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Start});
  _lib.com().mdu_ein();
  _state = &Update::modeResult;
}

void Update::modeResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Mode MDU_EIN" << std::endl;
    enterAction();
    return;
  }

  std::cout << "Unable to change mode" << std::endl;
  resetAction();
}

void Update::enterAction() {
  switch (_entryType) {
    case type::MDUEntryType::MDU: _lib.mdu_ein().enterMDU(); break;
    case type::MDUEntryType::DCC_ZSU:
      if (_decoderIDs.empty()) _lib.mdu_ein().enterDCCZSU();
      else _lib.mdu_ein().enterDCCZSU(*_iter, 0uz, _iter == _lastIter);
      break;
    default: assert(false);
  }

  _state = &Update::enterResult;
}

void Update::enterResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (_entryType == type::MDUEntryType::MDU) {
      // MDU entry, done after first command
      std::cout << "Entered via MDU" << std::endl;
      return configAction();
    }

    // DCC entry, use all ids first
    if (++_iter == _decoderIDs.end()) {
      // Done with last id, next mode
      std::cout << "Entered via DCC with " << _decoderIDs.size() << " IDs"
                << std::endl;
      return configAction();
    }

    // More IDs to send
    return enterAction();
  }

  std::cout << "Unable to enter" << std::endl;
  return resetAction();
}

void Update::configAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Init});
  _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Slow);
  _state = &Update::configResult;
}

void Update::configResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Data rate set to slow" << std::endl;
      searchAction();
      return;
    }
  }
  std::cout << "Unable to set data rate" << std::endl;
  resetAction();
}

void Update::searchAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::SearchDecoder});
  _lib.mdu_ein().ping(0, _fwIt.id());
  _state = &Update::searchResult;
}

void Update::searchResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Ping";
    if (std::get<res::Status>(r)) {
      std::cout << "Successful at Id 0x" << std::hex << _fwIt.id() << std::dec
                << std::endl;
      initAction();
      return;

    } else {
      std::cout << "Unsuccessful at Id 0x" << std::hex << _fwIt.id() << std::dec
                << std::endl;
      ++_fwIt;
      if (_fwIt == _fwItEnd) {
        std::cout << "No decoder found" << std::endl;
        resetAction();
        return;
      }

      searchAction();
      return;
    }
  }

  std::cout << "Unable to ping" << std::endl;
  resetAction();
}

void Update::initAction() {
  _lib.mdu_ein().zsuSalsa20Iv(_fwIt);
  _state = &Update::initResult;
}

void Update::initResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Salsa20 initialized" << std::endl;
      eraseAction();
      return;
    }
  }
  std::cout << "Unable to init Salsa20" << std::endl;
  resetAction();
  return;
}

void Update::eraseAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::EraseFlash});
  _lib.mdu_ein().zsuErase(_fwIt);
  _state = &Update::eraseResult;
}

void Update::eraseResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Erasing" << std::endl;
      waitAction();
      return;
    }
  }
  std::cout << "Unable to Erase" << std::endl;
  resetAction();
  return;
}

void Update::waitAction() {
  if (_updateCb)
    _updateCb({.id = type::MessageID::EraseFlash,
               .progress = static_cast<double>(_index) / 20.0});
  _lib.mdu_ein().busy();
  _state = &Update::waitResult;
}

void Update::waitResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (_index++ >= 20) {
      std::cout << "Finished erasing" << std::endl;
      _index = 0;
      updateAction();
      return;
    }
    std::cout << "Still erasing" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    waitAction();
    return;
  }

  std::cout << "Error while waiting for erasing" << std::endl;
  resetAction();
  return;
}

void Update::updateAction() {
  if (_updateCb)
    _updateCb({.id = type::MessageID::WriteFlash,
               .progress = static_cast<double>(_index + 1.0) /
                           static_cast<double>(_fwIt.blocks())});
  _lib.mdu_ein().zsuUpdate(_fwIt, _index);
  _state = &Update::updateResult;
}

void Update::updateResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      // Block transferred
      _err_cnt = 0;
      if (++_index >= _fwIt.blocks()) { return verifyAction(); }
      return updateAction();

    } else {
      // Block rejected
      _err_cnt++;
      if (_err_cnt < 3) return updateAction();

      // Too many errors
    }
  }
  std::cout << "Error while updating" << std::endl;
  resetAction();
}

void Update::verifyAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Verify});
  _lib.mdu_ein().zsuCrc32Start(_fwIt);
  _state = &Update::verifyResult;
}

void Update::verifyResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Started CRC32 verification" << std::endl;
      endAction();
      return;
    }
  }
  std::cout << "Unable to start verify" << std::endl;
  resetAction();
}

void Update::endAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Cleanup});
  _lib.mdu_ein().zsuCrc32ResultExit();
  _state = &Update::endResult;
}

void Update::endResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "CRC Success" << std::endl;
    } else {
      std::cout << "CRC Error" << std::endl;
    }
    resetAction();
    return;
  }
  std::cout << "Unable to verify" << std::endl;
  resetAction();
}

void Update::resetAction() {
  _lib.com().reset();
  _abort = false; // Else we'd loop forever on reset...
  _state = &Update::resetResult;
}

void Update::resetResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r))
    std::cout << "Reset success" << std::endl;
  else std::cout << "Reset NOT successful" << std::endl;

  if (_updateCb) _updateCb({.id = type::MessageID::Done});

  _done = true;
  return;
}

} // namespace process::mdu_ein
