/**
 *  Update process
 *
 * \file    src/process/mdu_ein/update.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "include/process/mdu_ein/update.hpp"
#include <iostream>
#include <thread>

namespace process::mdu_ein {

/**
 * CTor
 *
 * \param path        Path to ZSU file
 * \param entry_type  Entry type
 * \param decoder_ids List of decoder IDs (for entry)
 */
Update::Update(std::filesystem::path path,
               type::MDUEntryType entry_type,
               std::vector<uint32_t> decoder_ids)
  : Base{}, _zsu{std::make_shared<libklug::ZSU>(path)},
    _decoderIDs{decoder_ids}, _entryType{entry_type} {
  _lib.setCallback(
    [this](res::Result const result) { this->handle_result(result); });
}

/**
 * CTor
 *
 * \param path        ZSU file (LibKLUG)
 * \param entry_type  Entry type
 * \param decoder_ids List of decoder IDs (for entry)
 */
Update::Update(std::shared_ptr<libklug::ZSU> zsu,
               type::MDUEntryType entry_type,
               std::vector<uint32_t> decoder_ids)
  : Base{}, _zsu{zsu}, _decoderIDs{decoder_ids}, _entryType{entry_type} {
  assert(_zsu != nullptr);
  assert(_zsu->valid());

  _lib.setCallback(
    [this](res::Result const result) { this->handle_result(result); });
}

/**
 * DTor
 *
 */
Update::~Update() {
  _lib.unsetCallback();
  disconnect();
}

/**
 * Execute process
 *
 * \details
 * This process may not execute, if no device can be found.
 *
 * \return true   Process started
 * \return false  Unable to execute
 */
bool Update::execute() {
  if (_zsu == nullptr || !_zsu->valid() || !connect()) {
    _done = true;
    return false;
  }
  pingAction();
  return true;
}

/**
 * Abort process
 */
void Update::abort() { _abort = true; }

/**
 * Handle result (from callback)
 *
 * \note
 * If the process is to be aborted, \ref Update::resetAction is called
 * without handling the result.
 *
 * \param r Result (LibKLUG)
 */
void Update::handle_result(res::Result r) {
  if (_abort) resetAction();
  std::invoke(_state, this, r);
}

/**
 * Ping device
 *
 */
void Update::pingAction() {
  _lib.com().ping();
  _state = &Update::pingResult;
}

/**
 * Handle ping result
 *
 * \details
 * On success, the followup action is \ref Update::modeAction, else \ref
 * Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::pingResult(res::Result const r) {
  if (std::holds_alternative<res::String>(r)) {
    std::cout << "Found " << static_cast<std::string>(std::get<res::String>(r))
              << std::endl;
    if (_updateCb)
      _updateCb(
        {.id = type::MessageID::None,
         .payload = static_cast<std::string>(std::get<res::String>(r))});
    return modeAction();
  }
}

/**
 * Change mode action
 *
 */
void Update::modeAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Start});
  _lib.com().mdu_ein();
  _state = &Update::modeResult;
}

/**
 * Handle change mode result
 *
 * \details
 * On success, the followup action is \ref Update::enterAction, else \ref
 * Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::modeResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Mode MDU_EIN" << std::endl;
    enterAction();
    return;
  }

  std::cout << "Unable to change mode" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  resetAction();
}

/**
 * Enter
 *
 * \note
 * Entry used depends on value given in CTor
 *
 */
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

/**
 * Handle enter result
 *
 * \details
 * On success, the followup action is \ref Update::configAction, else \ref
 * Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::enterResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (_entryType == type::MDUEntryType::MDU) {
      // MDU entry, done after first command
      std::cout << "Entered via MDU" << std::endl;
      return configAction();
    }

    // DCC entry, use all ids first
    if (_decoderIDs.empty() || ++_iter == _decoderIDs.end()) {
      // Done with last id, next mode
      std::cout << "Entered via DCC with " << _decoderIDs.size() << " IDs"
                << std::endl;
      return configAction();
    }

    // More IDs to send
    return enterAction();
  }

  std::cout << "Unable to enter" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  return resetAction();
}

/**
 * Config (transfer rate)
 *
 * \todo Maybe don't abort if we can't go fast
 *
 */
void Update::configAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Init});
  _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Slow);
  _state = &Update::configResult;
}

/**
 * Handle config result
 *
 * \details
 * On success, the followup action is \ref Update::searchAction, else \ref
 * Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::configResult(res::Result const r) {
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

/**
 * Search decoder
 *
 * \note
 * Since we don't exacly have an ID list, we just ping 0 and check if something
 * responds
 *
 */
void Update::searchAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::SearchDecoder});
  _lib.mdu_ein().ping(0, _fwIt.id());
  _state = &Update::searchResult;
}

/**
 * Handle search result
 *
 * \details
 * This searches for all decoders with available firmware, with the ID being a
 * byproduct of the FirmwareIterator. On success, the followup action is \ref
 * Update::initAction, on any error, the next ID is searched.
 *
 * If no more IDs are available, the next action is \ref Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
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
        _updateCb({.id = type::MessageID::AbortDecoderSearch, .payload = true});
        resetAction();
        return;
      }

      searchAction();
      return;
    }
  }

  std::cout << "Unable to ping" << std::endl;
  _updateCb({.id = type::MessageID::AbortDecoderSearch, .payload = true});
  resetAction();
}

/**
 * Init (Salsa20)
 *
 */
void Update::initAction() {
  _lib.mdu_ein().zsuSalsa20Iv(_fwIt);
  _state = &Update::initResult;
}

/**
 * Handle init result
 *
 * \details
 * On success, the followup action is \ref Update::eraseAction, else \ref
 * Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::initResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Salsa20 initialized" << std::endl;
      eraseAction();
      return;
    }
  }
  std::cout << "Unable to init Salsa20" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  resetAction();
  return;
}

/**
 * Erase flash
 *
 */
void Update::eraseAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::EraseFlash});
  _lib.mdu_ein().zsuErase(_fwIt);
  _state = &Update::eraseResult;
}

/**
 * Handle enter result
 *
 * \details
 * On success, the followup action is \ref Update::waitAction, else \ref
 * Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::eraseResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Erasing" << std::endl;
      waitAction();
      return;
    }
  }
  std::cout << "Unable to Erase" << std::endl;
  _updateCb({.id = type::MessageID::AbortFlashErase, .payload = true});
  resetAction();
  return;
}

/**
 * Wait for erase finish
 *
 * \note This will essentially get looped from \ref Update::waitResult until
 * erase is done
 *
 */
void Update::waitAction() {
  if (_updateCb)
    _updateCb({.id = type::MessageID::EraseFlash,
               .progress = static_cast<double>(_index) / 20.0});
  _lib.mdu_ein().busy();
  _state = &Update::waitResult;
}

/**
 * Handle enter result
 *
 * \details
 * This needs to 'loop' for about 10 seconds. While these 10 seconds have not
 * passed, the next action will always be \ref Update::waitAction. Once the time
 * has passed the next action is \ref Update::updateAction
 *
 * \param r Result (LibKLUG)
 */
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
  _updateCb({.id = type::MessageID::AbortFlashErase, .payload = true});
  resetAction();
  return;
}

/**
 * Update (write flash block)
 *
 */
void Update::updateAction() {
  if (_updateCb)
    _updateCb({.id = type::MessageID::WriteFlash,
               .progress = static_cast<double>(_index + 1.0) /
                           static_cast<double>(_fwIt.blocks())});
  _lib.mdu_ein().zsuUpdate(_fwIt, _index);
  _state = &Update::updateResult;
}

/**
 * Handle update result
 *
 * \details
 * On Success, either the next block is written with \ref
 * Update::updateAction, or, in case the last block was successfully sent,
 * \ref Update::verifyAction.
 *
 * On Error, the current block is retried up to 3 times, then \ref
 * Update::resetAction is executed.
 *
 * \param r Result (LibKLUG)
 */
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
  _updateCb({.id = type::MessageID::AbortFlashWrite, .payload = true});
  resetAction();
}

/**
 * Verify update (start CRC32 verification)
 *
 */
void Update::verifyAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Verify});
  _lib.mdu_ein().zsuCrc32Start(_fwIt);
  _state = &Update::verifyResult;
}

/**
 * Handle verify result
 *
 * \details
 * On success, the followup action is \ref Update::endAction, else \ref
 * Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::verifyResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Started CRC32 verification" << std::endl;
      endAction();
      return;
    }
  }
  std::cout << "Unable to start verify" << std::endl;
  _updateCb({.id = type::MessageID::AbortVerify, .payload = true});
  resetAction();
}

/**
 * End (cleanup)
 *
 */
void Update::endAction() {
  if (_updateCb) _updateCb({.id = type::MessageID::Cleanup});
  _lib.mdu_ein().zsuCrc32ResultExit();
  _state = &Update::endResult;
}

/**
 * Handle end result
 *
 * \details
 * In any case, the followup action is \ref Update::resetAction
 *
 * \param r Result (LibKLUG)
 */
void Update::endResult(res::Result const r) {
  _updateCb({.id = type::MessageID::Done, .payload = true});
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

/**
 * Reset
 *
 */
void Update::resetAction() {
  _lib.com().reset();
  if (_abort) {
    _updateCb({.id = type::MessageID::Abort, .payload = true});
    _abort = false; // Else we'd loop forever on reset...
  }
  _state = &Update::resetResult;
}

/**
 * Handle reset result
 *
 * \details
 * Either success or no succes, this is the last action.
 *
 * \param r Result (LibKLUG)
 */
void Update::resetResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r))
    std::cout << "Reset success" << std::endl;
  else std::cout << "Reset NOT successful" << std::endl;

  _done = true;
  return;
}

} // namespace process::mdu_ein
