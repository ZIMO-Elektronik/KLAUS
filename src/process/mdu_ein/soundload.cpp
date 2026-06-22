#include "include/process/mdu_ein/soundload.hpp"
#include <iostream>
#include <thread>
#include "include/type/step.hpp"

namespace process::mdu_ein {

SoundLoad::SoundLoad(std::filesystem::path path) : _zpp{path} {
  _lib.registerCb<[] {}>(
    [this](res::Result const result) { this->handle_result(result); });
}

SoundLoad::~SoundLoad() {
  _lib.deregisterCb();
  disconnect();
}

bool SoundLoad::execute() {
  if (!connect()) {
    _done = true;
    return false;
  }
  modeAction();
  return true;
}

void SoundLoad::abort() { _abort = true; }

void SoundLoad::handle_result(res::Result r) {
  if (_abort) resetAction();
  std::invoke(_state, this, r);
}

void SoundLoad::modeAction() {
  _lib.com().mdu_ein();
  _state = &SoundLoad::modeResult;
}

void SoundLoad::modeResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Mode MDU_EIN" << std::endl;
    enterAction();
    return;
  }

  std::cout << "Unable to change mode" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  resetAction();
}

void SoundLoad::enterAction() {
  _lib.mdu_ein().enterDCCZPP();
  _state = &SoundLoad::enterResult;
}

void SoundLoad::enterResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Entered via MDU" << std::endl;
    configAction();
    return;
  }

  std::cout << "Unable to enter" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  resetAction();
}

void SoundLoad::configAction() {
  _updateCb({.id = type::MessageID::Init});
  _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Fast);
  _state = &SoundLoad::configResult;
}

void SoundLoad::configResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Data rate set to slow" << std::endl;
      searchAction();
      return;
    }
  }
  std::cout << "Unable to set data rate" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  resetAction();
}

void SoundLoad::searchAction() {
  _updateCb({.id = type::MessageID::SearchDecoder});
  _lib.mdu_ein().ping(0uz, 0uz);
  _state = &SoundLoad::searchResult;
}

void SoundLoad::searchResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Ping";
    if (std::get<res::Status>(r)) {
      _err_cnt = 0;
      std::cout << "Successful at Id 0x" << std::hex << 0uz << std::dec
                << std::endl;
      initAction();
      return;

    } else {
      std::cout << "Unsuccessful at Id 0x" << std::hex << 0uz << std::dec
                << std::endl;

      if (_err_cnt++ < 3) {
        // 3 retries
        return searchAction();
      }
    }
  }

  std::cout << "Unable to ping" << std::endl;
  _updateCb({.id = type::MessageID::AbortDecoderSearch, .payload = true});
  resetAction();
}

void SoundLoad::initAction() {
  _updateCb({.id = type::MessageID::Init});
  _lib.mdu_ein().zppValidQuery(_zpp);
  _state = &SoundLoad::initResult;
}

void SoundLoad::initResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "ZPP valid" << std::endl;
      eraseAction();
      return;
    }
  }
  std::cout << "Unable to check if ZPP is valid" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  resetAction();
  return;
}

void SoundLoad::eraseAction() {
  _updateCb({.id = type::MessageID::EraseFlash});
  _lib.mdu_ein().zppErase(_zpp);
  _state = &SoundLoad::eraseResult;
}

void SoundLoad::eraseResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Erasing..." << std::endl;
      return waitAction();
    }
  }
  std::cout << "Unable to erase flash" << std::endl;
  _updateCb({.id = type::MessageID::AbortFlashErase, .payload = true});
  resetAction();
  return;
}

void SoundLoad::waitAction() {
  _updateCb({.id = type::MessageID::EraseFlash,
             .progress = static_cast<double>(_index) / 200.0});
  _lib.mdu_ein().busy();
  _state = &SoundLoad::waitResult;
}

void SoundLoad::waitResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      _index = 0;
      std::cout << "Erased" << std::endl;
      return updateAction();
    }
    _index++;
    std::cout << "Still erasing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return waitAction();
  }
  std::cout << "Unable to wait for erase complete" << std::endl;
  _updateCb({.id = type::MessageID::AbortFlashErase, .payload = true});
  return resetAction();
}

void SoundLoad::updateAction() {
  _updateCb({.id = type::MessageID::WriteFlash,
             .progress{static_cast<double>(_index + 1.0) /
                       static_cast<double>(_zpp.blocks())}});
  _lib.mdu_ein().zppUpdate(_zpp, _index);
  _state = &SoundLoad::updateResult;
}

void SoundLoad::updateResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      // Block written
      _err_cnt = 0;
      if (++_index >= _zpp.blocks()) { return endAction(); }
      return updateAction();
    }
    // Block rejected
    if (_err_cnt++ < 3) { return updateAction(); }
  }
  std::cout << "Unable to write flash" << std::endl;
  _updateCb({.id = type::MessageID::AbortFlashWrite, .payload = true});
  return resetAction();
}

void SoundLoad::endAction() {
  _updateCb({.id = type::MessageID::Cleanup});
  _lib.mdu_ein().zppUpdateEnd(_zpp);
  _state = &SoundLoad::endResult;
}

void SoundLoad::endResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) { return exitAction(); }
  }
  std::cout << "Unable to perform updateEnd" << std::endl;
  _updateCb({.id = type::MessageID::AbortFlashErase, .payload = true});
  return resetAction();
}

void SoundLoad::exitAction() {
  _lib.mdu_ein().zppExitReset();
  _state = &SoundLoad::exitResult;
}

void SoundLoad::exitResult(res::Result const r) {
  _updateCb({.id = type::MessageID::Done});
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) { return resetAction(); }
  }
  std::cout << "Unable to perform exitReset" << std::endl;
  return resetAction();
}

void SoundLoad::resetAction() {
  _lib.com().reset();
  if (_abort) {
    _updateCb({.id = type::MessageID::Abort, .payload = true});
    _abort = false; // Else we'd loop forever on reset...
  }
  _state = &SoundLoad::resetResult;
}
void SoundLoad::resetResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r))
    std::cout << "Reset success" << std::endl;
  else std::cout << "Reset NOT successful" << std::endl;

  _done = true;
  return;
}

} // namespace process::mdu_ein
