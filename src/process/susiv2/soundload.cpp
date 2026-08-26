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
 */
SoundLoad::SoundLoad(std::filesystem::path path)
  : _zpp{std::make_shared<libklug::ZPP>(path)} {}

/**
 * CTor
 *
 * \param zpp Shared pointer to ZPP
 */
SoundLoad::SoundLoad(std::shared_ptr<libklug::ZPP> zpp) : _zpp{zpp} {}

/**
 * DTor
 *
 */
SoundLoad::~SoundLoad() { disconnect(); }

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
  if (_zpp == nullptr || !_zpp->valid() || !connect()) {
    _done = true;
    return false;
  }
  _process = std::async([this]() { return this->load(); });
  return true;
}

/**
 * The actual SoundLoad
 *
 * \details
 * This will execute components in imperative order until either completion, or
 * the first non-recoverable Error. Either way, the SoundLoad will be ended
 * cleanly and the Update device is reset.
 *
 */
void SoundLoad::load() {
  if (!ping() || !mode() || !features() || !erase() || !write()) _abort = true;
  exit();
  reset();
}

/**
 * Change mode to SUSIV2
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::mode() {
  pushUI({.id = type::MessageID::Start});
  if (auto const res{_lib.com().susiv2()})
    if (*res) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Request decoder features
 *
 * \note
 * Actually, this sets the max transfer speed available
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::features() {
  if (auto const res{_lib.susiv2().features()})
    if (*res) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Erase decoder flash (and wait until complete)
 *
 * \todo
 * This should update the UI while erasing. Maybe defer this to another thread
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::erase() {
  pushUI({.id = type::MessageID::EraseFlash});

  if (auto const res{_lib.susiv2().zppErase()})
    if (*res) return true;

  pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
  return false;
}

/**
 * Write Decoder flash
 *
 * \details
 * This writes the actual update into the decoder flash. A single block is
 * retried up to 3 times, if it still fails, we abort with `false`. Otherwise,
 * all blocks are transmitted here.
 *
 * \todo
 * Perhaps we could retry the previous 2 blocks before aborting.
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::write() {
  unsigned int index{0u};

  while (!_abort) {
    pushUI({.id = type::MessageID::WriteFlash,
            .progress{static_cast<double>(index + 1.0) /
                      static_cast<double>(_zpp->blocks())}});
    if (auto const res{_lib.susiv2().zppWrite(*_zpp, index)}) {
      if (*res) { // Block written
        _err_cnt = 0;
        if (++index >= _zpp->blocks()) // Done
          return true;

      } else if (_err_cnt++ >= 3uz) { // Max errors reached
        pushUI({.id = type::MessageID::AbortFlashWrite, .payload = true});
        break;
      }
    }
  }

  // Aborted
  return false;
}

/**
 * Exit ZUSI mode (for decoder)
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::exit() {
  if (auto const res{_lib.susiv2().exit(true, true)})
    if (*res) return true;

  return false;
}

} // namespace process::susiv2
