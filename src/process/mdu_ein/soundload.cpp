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
  if (!ping() || !mode() || !enter() || !config() || !search() || !init() ||
      !erase() || !write() || !end())
    _abort = true;

  // There was a reason for this
  _lib.mdu_ein().busy();
  exit();
  _lib.mdu_ein().busy();
  reset();
}

/**
 * Change mode to MDU_EIN
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::mode() {
  pushUI({.id = type::MessageID::Start});

  if (auto const res{_lib.com().mdu_ein()})
    if (*res) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Enter Decoder(-s)
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::enter() {
  if (auto const res{_lib.mdu_ein().enterDCCZPP()})
    if (*res) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Configure Transfer Rate
 *
 * \details
 * This configures the Transfer rate to `Fast`.
 *
 * \todo
 * Maybe this should retry with a slower rate on fail
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::config() {
  pushUI({.id = type::MessageID::Init});

  if (auto const res{
        _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Fast)})
    if (*res) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Search decoder
 *
 * \note
 * Since we don't exacly have an ID list, we just ping 0 and check if something
 * responds
 *
 */
bool SoundLoad::search() {
  _err_cnt = 0uz;
  pushUI({.id = type::MessageID::SearchDecoder});

  while (_err_cnt++ < 3uz)
    if (auto const res{_lib.mdu_ein().ping(0uz, 0uz)})
      if (*res) return true;

  pushUI({.id = type::MessageID::AbortDecoderSearch, .payload = true});
  return false;
}

/**
 * Check if the ZPP can fit into the decoder
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::init() {
  if (auto const res{_lib.mdu_ein().zppValidQuery(*_zpp)})
    if (*res) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Erase decoder flash (and wait until done)
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::erase() {
  // Erase flash
  if (auto const res{_lib.mdu_ein().zppErase(*_zpp)}) {
    if (!*res) { // Can't erase
      pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
      return false;
    }
  } else {
    pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
    return false;
  }

  // Wait until flash is erased
  unsigned int index{0u};
  while (!_abort) {
    pushUI({.id = type::MessageID::EraseFlash,
            .progress = static_cast<double>(index++) / 200.0});
    if (auto const res{_lib.mdu_ein().busy()}) {
      if (*res) { // Done
        return true;
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
    break;
  }

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
 * \todo
 * Maybe we should ping the decoder sometimes to check if it still exists
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
    if (auto const res{_lib.mdu_ein().zppUpdate(*_zpp, index)}) {
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
 * Formally end sound load
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::end() {
  if (auto const res{_lib.mdu_ein().zppUpdateEnd(*_zpp)})
    if (*res) return true;

  std::cout << "Failed to end ZPP Update" << std::endl;
  pushUI({.id = type::MessageID::AbortVerify, .payload = true});
  return false;
}

/**
 * Exit sound load (for decoder)
 *
 * \return true   Continue
 * \return false  Abort
 */
bool SoundLoad::exit() {
  if (auto const res{_lib.mdu_ein().zppExitReset()})
    if (*res) return true;

  return false;
}

} // namespace process::mdu_ein
