/**
 * SoundLoad process
 *
 * \file    src/process/susiv2/soundload.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "include/process/susiv2/soundload.hpp"
#include <iostream>

namespace process::susiv2 {

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
  if (_abort) return resetAction();
  std::invoke(_state, this, r);
}

/**
 * Change mode action
 *
 */
void SoundLoad::modeAction() {
  _updateCb({.id = type::MessageID::Start});
  _state = &SoundLoad::modeResult;
  _lib.com().susiv2();
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
void SoundLoad::modeResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Entered SUSIV2" << std::endl;
      return featuresAction();
    }
  }

  std::cout << "Unable to enter SUSIV2" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  return resetAction();
}

/**
 * Features action (also sets transfer speed implicitly)
 *
 */
void SoundLoad::featuresAction() {
  _updateCb({.id = type::MessageID::Init});
  _state = &SoundLoad::featuresResult;
  _lib.susiv2().features();
}

/**
 * Handle features result
 *
 * \note The attached data is not evaluated, this is more of a (do i have a
 * decoder?)
 *
 * \param r Result (LibKLUG)
 */
void SoundLoad::featuresResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Requested features" << std::endl;
      return eraseAction();
    }
  }

  std::cout << "Unable to request features" << std::endl;
  _updateCb({.id = type::MessageID::AbortInit, .payload = true});
  return resetAction();
}

/**
 * Erase flash
 *
 */
void SoundLoad::eraseAction() {
  _updateCb({.id = type::MessageID::EraseFlash});
  _state = &SoundLoad::eraseResult;
  _lib.susiv2().zppErase();
}

/**
 * Handle erase result
 *
 * \details
 * On success, the followup action is \ref SoundLoad::loadAction, else \ref
 * SoundLoad::resetAction
 *
 * \note
 * The main drawback of this approach is that its blocking. which means we'd
 * need a thread to update the UI.
 *
 * \param r Result (LibKLUG)
 */
void SoundLoad::eraseResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Erased flash" << std::endl;
      return loadAction();
    }
  }

  std::cout << "Unable to erase flash" << std::endl;
  _updateCb({.id = type::MessageID::AbortFlashErase, .payload = true});
  return resetAction();
}

/**
 * Load (write flash block)
 *
 */
void SoundLoad::loadAction() {
  _updateCb({.id = type::MessageID::WriteFlash,
             .progress = static_cast<double>(_index + 1.0) /
                         static_cast<double>(_zpp.blocks())});
  _state = &SoundLoad::loadResult;
  _lib.susiv2().zppWrite(_zpp, _index);
}

/**
 * Handle load result
 *
 * \details
 * On Success, either the next block is written with \ref
 * SoundLoad::loadAction, or, in case the last block was successfully sent,
 * \ref SoundLoad::endAction.
 *
 * On Error, the current block is retried up to 3 times, then \ref
 * SoundLoad::resetAction is executed.
 *
 * \param r Result (LibKLUG)
 */
void SoundLoad::loadResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      // Block written
      _err_cnt = 0;
      if (++_index >= _zpp.blocks()) { return endAction(); }
      return loadAction();
    } else {
      // Block rejected
      if (_err_cnt++ < 3) {
        // Retry
        return loadAction();
      }

      // Too many errors
    }
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
  _state = &SoundLoad::endResult;
  _lib.susiv2().exit(true, true);
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
void SoundLoad::endResult(res::Result const& r) {
  _updateCb({.id = type::MessageID::Done, .payload = true});
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Exited" << std::endl;
      return resetAction();
    }
  }

  std::cout << "Unable to exit" << std::endl;
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
void SoundLoad::resetResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r))
    std::cout << "Reset success" << std::endl;
  else std::cout << "Reset NOT successful" << std::endl;

  _done = true;
  return;
}

} // namespace process::susiv2
