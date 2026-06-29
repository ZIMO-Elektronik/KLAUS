/**
 * SoundLoad process
 *
 * \file    src/process/mdu_ein/soundload.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "include/process/mdu_ein/soundload.hpp"
#include <iostream>
#include <thread>
#include "include/type/step.hpp"

namespace process::mdu_ein {

/**
 * CTor
 *
 * \param path Path to ZPP file
 *
 * \todo Make ZPP a shared_ptr and add a CTor taking that
 */
SoundLoad::SoundLoad(std::filesystem::path path) : _zpp{path} {
  _lib.setCallback(
    [this](res::Result const result) { this->handle_result(result); });
}

/**
 * DTor
 *
 */
SoundLoad::~SoundLoad() {
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
bool SoundLoad::execute() {
  if (!connect()) {
    _done = true;
    return false;
  }
  modeAction();
  return true;
}

/**
 * Abort process
 */
void SoundLoad::abort() { _abort = true; }

/**
 * Handle result (from callback)
 *
 * \note
 * If the process is to be aborted, \ref SoundLoad::resetAction is called
 * without handling the result.
 *
 * \param r Result (LibKLUG)
 */
void SoundLoad::handle_result(res::Result r) {
  if (_abort) resetAction();
  std::invoke(_state, this, r);
}

/**
 * Change mode action
 *
 */
void SoundLoad::modeAction() {
  _lib.com().mdu_ein();
  _state = &SoundLoad::modeResult;
}

/**
 * Handle change mode result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::enterAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * Enter
 *
 * \note
 * Since were loading sound, we lock the entry to DCCZPP
 *
 */
void SoundLoad::enterAction() {
  _lib.mdu_ein().enterDCCZPP();
  _state = &SoundLoad::enterResult;
}

/**
 * Handle enter result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::configAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * Config (transfer rate)
 *
 * \todo Maybe don't abort if we can't go fast
 *
 */
void SoundLoad::configAction() {
  _updateCb({.id = type::MessageID::Init});
  _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Fast);
  _state = &SoundLoad::configResult;
}

/**
 * Handle config result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::searchAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * Search decoder
 *
 * \note
 * Since we don't exacly have an ID list, we just ping 0 and check if something
 * responds
 *
 */
void SoundLoad::searchAction() {
  _updateCb({.id = type::MessageID::SearchDecoder});
  _lib.mdu_ein().ping(0uz, 0uz);
  _state = &SoundLoad::searchResult;
}

/**
 * Handle search result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::initAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * Init (check zppValid)
 *
 */
void SoundLoad::initAction() {
  _updateCb({.id = type::MessageID::Init});
  _lib.mdu_ein().zppValidQuery(_zpp);
  _state = &SoundLoad::initResult;
}

/**
 * Handle init result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::eraseAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * Erase flash
 *
 */
void SoundLoad::eraseAction() {
  _updateCb({.id = type::MessageID::EraseFlash});
  _lib.mdu_ein().zppErase(_zpp);
  _state = &SoundLoad::eraseResult;
}

/**
 * Handle change mode result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::waitAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * Wait for erase finish
 *
 * \note This will essentially get looped from \ref SoundLoad::waitResult until
 * erase is done
 *
 */
void SoundLoad::waitAction() {
  _updateCb({.id = type::MessageID::EraseFlash,
             .progress = static_cast<double>(_index) / 200.0});
  _lib.mdu_ein().busy();
  _state = &SoundLoad::waitResult;
}

/**
 * Handle change mode result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::enterAction.
 *
 * On no_success (busy), the followup action is \ref SoundLoad::waitAction.
 *
 * On any error, the followup action is \ref SoundLoad::resetAction.
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * Update (write flash block)
 *
 */
void SoundLoad::updateAction() {
  _updateCb({.id = type::MessageID::WriteFlash,
             .progress{static_cast<double>(_index + 1.0) /
                       static_cast<double>(_zpp.blocks())}});
  _lib.mdu_ein().zppUpdate(_zpp, _index);
  _state = &SoundLoad::updateResult;
}

/**
 * Handle update result
 *
 * \details
 * On Success, either the next block is written with \ref
 * SoundLoad::updateAction, or, in case the last block was successfully sent,
 * \ref SoundLoad::endAction.
 *
 * On Error, the current block is retried up to 3 times, then \ref
 * SoundLoad::resetAction is executed.
 *
 * \param r Result (LibKLUG)
 */
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

/**
 * End (cleanup)
 *
 */
void SoundLoad::endAction() {
  _updateCb({.id = type::MessageID::Cleanup});
  _lib.mdu_ein().zppUpdateEnd(_zpp);
  _state = &SoundLoad::endResult;
}

/**
 * Handle end result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::exitAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
void SoundLoad::endResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) { return exitAction(); }
  }
  std::cout << "Unable to perform updateEnd" << std::endl;
  _updateCb({.id = type::MessageID::AbortFlashErase, .payload = true});
  return resetAction();
}

/**
 * Exit (decoder exit from MDU)
 *
 */
void SoundLoad::exitAction() {
  _lib.mdu_ein().zppExitReset();
  _state = &SoundLoad::exitResult;
}

/**
 * Handle exit result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::exitAction, else \ref
 * SoundLoad::resetAction
 *
 * \param r Result (LibKLUG)
 */
void SoundLoad::exitResult(res::Result const r) {
  _updateCb({.id = type::MessageID::Done});
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) { return resetAction(); }
  }
  std::cout << "Unable to perform exitReset" << std::endl;
  return resetAction();
}

/**
 * Reset
 *
 */
void SoundLoad::resetAction() {
  _lib.com().reset();
  if (_abort) {
    _updateCb({.id = type::MessageID::Abort, .payload = true});
    _abort = false; // Else we'd loop forever on reset...
  }
  _state = &SoundLoad::resetResult;
}

/**
 * Handle reset result
 *
 * \details
 * Either success or no succes, this is the last action.
 *
 * \param r Result (LibKLUG)
 */
void SoundLoad::resetResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r))
    std::cout << "Reset success" << std::endl;
  else std::cout << "Reset NOT successful" << std::endl;

  _done = true;
  return;
}

} // namespace process::mdu_ein
