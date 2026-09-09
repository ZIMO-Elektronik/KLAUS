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
#include "include/process/process_error.hpp"
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
  try {
    try { // Acual Update
      ping();
      mode();
      enter();
      config();
      search();
      init();
      erase();
      write();
      end();
    } catch (process_error const& e) {
      std::cerr << e.what();
      pushUI(static_cast<type::ProcessUpdate>(e));
    }

    // Finalize
    exit();
    reset();
  } catch (std::exception const& e) {
    std::cerr << e.what();
    pushUI({.id = type::MessageID::AbortUnresponsive, .payload = true});
  }
}

/**
 * Change mode to MDU_EIN
 *
 * \throws process_error  If the mode is unavailable
 * \throws klug_error     If the communication failed
 */
void SoundLoad::mode() {
  pushUI({.id = type::MessageID::Start});

  if (!_lib.com().mdu_ein())
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to change mode"};
}

/**
 * Enter Decoder(-s)
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::enter() {
  if (!_lib.com().mdu_ein())
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to enter decoders"};
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
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::config() {
  pushUI({.id = type::MessageID::Init});

  if (_lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Fast))
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to configure transfer rate"};
}

/**
 * Search decoder
 *
 * \note
 * Since we don't exacly have an ID list, we just ping 0 and check if
 * something responds
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 *
 */
void SoundLoad::search() {
  pushUI({.id = type::MessageID::SearchDecoder});

  for (int i{}; i < 5; i++) {
    if (_lib.mdu_ein().ping(0uz, 0uz)) { return; }
  }

  throw process_error{
    {.id = type::MessageID::AbortDecoderSearch, .payload = true},
    "No decoder found"};
  _err_cnt = 0uz;
}

/**
 * Check if the ZPP can fit into the decoder
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::init() {
  if (!_lib.mdu_ein().zppValidQuery(*_zpp))
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Not enough space in flash"};
}

/**
 * Erase decoder flash (and wait until done)
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::erase() {
  // Erase flash
  if (_lib.mdu_ein().zppErase(*_zpp))
    throw process_error{
      {.id = type::MessageID::AbortFlashErase, .payload = true},
      "Unable to erase decoder flash"};

  // Wait until flash is erased
  unsigned int index{0u};
  while (!_abort) {
    pushUI({.id = type::MessageID::EraseFlash,
            .progress = static_cast<double>(index++) / 200.0});

    if (_lib.mdu_ein().busy()) break;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
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
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::write() {
  unsigned int index{0u};

  for (int index{0}; index < _zpp->blocks(); index++) {
    checkAbort();

    pushUI({.id = type::MessageID::WriteFlash,
            .progress{static_cast<double>(index + 1.0) /
                      static_cast<double>(_zpp->blocks())}});

    if (index % 64 == 0) lifesign();

    int tries{0};
    do {
      if (_lib.mdu_ein().zppUpdate(*_zpp, index)) break;
    } while (++tries < 3);

    // Check if we have reached max retries
    if (tries >= 3)
      throw process_error{
        {.id = type::MessageID::AbortFlashWrite, .payload = true},
        "Too many consecutive errors"};

    // Check if done
    if (index >= _zpp->blocks()) break;
  }
}

/**
 * Formally end sound load
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::end() {
  if (!_lib.mdu_ein().zppUpdateEnd(*_zpp))
    throw process_error{{.id = type::MessageID::AbortVerify, .payload = true},
                        "Failed to end process orderly"};
}

/**
 * Exit sound load (for decoder)
 *
 * \throws klug_error     If the communication failed
 */
void SoundLoad::exit() { _lib.mdu_ein().zppExitReset(); }

/**
 * Pings all decoders to check if any answers
 *
 */
void SoundLoad::lifesign() {
  int tries{0};
  do {
    if (_lib.mdu_ein().ping(0u, 0u)) break;
  } while (tries < 3);

  if (tries >= 3)
    throw process_error{
      {.id = type::MessageID::AbortFlashWrite, .payload = true},
      "Decoder not pingable"};
}

/**
 * Checks if the process should be aborted
 *
 * \throws process_error If the process was aborted
 */
void SoundLoad::checkAbort() {
  if (_abort)
    throw process_error{{.id = type::MessageID::Abort, .payload = true},
                        "Process Aborted"};
}

} // namespace process::mdu_ein
