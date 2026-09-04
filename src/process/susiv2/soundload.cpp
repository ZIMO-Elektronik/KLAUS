/**
 * SoundLoad process
 *
 * \file    src/process/susiv2/soundload.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "include/process/susiv2/soundload.hpp"
#include <iostream>
#include "include/process/process_error.hpp"

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
  try {
    try { // Actual Update
      ping();
      mode();
      features();
      erase();
      write();
    } catch (process_error const& e) {
      pushUI(static_cast<type::ProcessUpdate>(e));
    }

    // Finalize
    exit();
    reset();
  } catch (...) {
    pushUI({.id = type::MessageID::AbortUnresponsive, .payload = true});
  }
}

/**
 * Change mode to SUSIV2
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::mode() {
  pushUI({.id = type::MessageID::Start});

  if (!_lib.com().susiv2())
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to change mode"};
}

/**
 * Request decoder features
 *
 * \note
 * Actually, this sets the max transfer speed available
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::features() {
  if (!_lib.susiv2().features())
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to request features"};
}

/**
 * Erase decoder flash (and wait until complete)
 *
 * \todo
 * This should update the UI while erasing. Maybe defer this to another thread
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::erase() {
  pushUI({.id = type::MessageID::EraseFlash});

  if (!_lib.susiv2().zppErase())
    throw process_error{
      {.id = type::MessageID::AbortFlashErase, .payload = true},
      "Unable to erase flash"};
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
  for (unsigned int index{0}, max_index{_zpp->blocks()}; index < max_index;
       index++) {
    checkAbort();

    pushUI({.id = type::MessageID::WriteFlash,
            .progress{static_cast<double>(index + 1.0) /
                      static_cast<double>(_zpp->blocks())}});

    int tries{0};
    do {
      if (_lib.susiv2().zppWrite(*_zpp, index)) break;
    } while (++tries < 3);

    if (tries >= 3)
      throw process_error{
        {.id = type::MessageID::AbortFlashWrite, .payload = true},
        "Unable to write flash"};
  }
}

/**
 * Exit ZUSI mode (for decoder)
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void SoundLoad::exit() {
  if (!_lib.susiv2().exit(true, true))
    throw process_error{{.id = type::MessageID::AbortVerify, .payload = true},
                        "Unable to finalize soundload"};
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

} // namespace process::susiv2
