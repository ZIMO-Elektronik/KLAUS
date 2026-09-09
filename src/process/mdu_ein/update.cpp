/**
 *  Update process
 *
 * \file    src/process/mdu_ein/update.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "include/process/mdu_ein/update.hpp"
#include <algorithm>
#include <future>
#include <iostream>
#include <thread>
#include "include/process/process_error.hpp"

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
    _decoderIDs{decoder_ids}, _entryType{entry_type} {}

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
}

/**
 * DTor
 *
 */
Update::~Update() { disconnect(); }

/**
 * Execute process
 *
 * \return true   Process running
 * \return false  Error during setup
 */
bool Update::execute() {
  if (_zsu == nullptr || !_zsu->valid() || !connect()) {
    _done = true;
    return false;
  }

  _process = std::async([this]() { return this->update(); });
  return true;
}

/**
 * The actual update
 *
 * \details
 * This will execute components in imperative order until either completion, or
 * the first non-recoverable Error. Either way, the Update will be ended cleanly
 * and the Update device is reset.
 */
void Update::update() {
  try {
    try { // Actual Update
      ping();
      mode();
      enter();
      config();
      search();
      init();
      erase();
      write();
      verify();
    } catch (process_error const& e) {
      std::cerr << e.what() << std::endl;
      pushUI(static_cast<type::ProcessUpdate>(e));
    }

    // Finalize
    end();
    reset();
  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
    pushUI({.id = type::MessageID::AbortUnresponsive, .payload = true});
  }
}

/**
 * Change mode to MDU_EIN
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void Update::mode() {
  pushUI({.id = type::MessageID::Start});
  if (!_lib.com().mdu_ein())
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to change mode"};
}

/**
 * Enter Decoder(-s)
 *
 * \details
 * This will perform the selected entry in \ref Update::_entryType. In case of a
 * `DCC_ZSU` entry, all decoders in the given \ref Update::_decoderIDs are
 * entered, in case it is empty, entry is done with ID and SN = 0.
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void Update::enter() {
  assert(_entryType == type::MDUEntryType::MDU ||
         _entryType == type::MDUEntryType::DCC_ZSU);

  if (_abort)
    throw process_error{{.id = type::MessageID::Abort, .payload = true},
                        "Process Aborted"};

  switch (_entryType) {
    case type::MDUEntryType::MDU:
      if (!_lib.mdu_ein().enterMDU())
        throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                            "Unable to enter decoders"};
      break;
    case type::MDUEntryType::DCC_ZSU:
      if (_decoderIDs.empty()) {
        if (!_lib.mdu_ein().enterDCCZSU())
          throw process_error{
            {.id = type::MessageID::AbortInit, .payload = true},
            "Unable to enter decoders"};
      } else {
        auto iter{_decoderIDs.begin()};
        do {
          if (!_lib.mdu_ein().enterDCCZSU(
                *iter, 0uz, ++iter == _decoderIDs.end()))
            throw process_error{
              {.id = type::MessageID::AbortInit, .payload = true},
              "Unable to enter decoders"};

        } while (iter != _decoderIDs.end());
      }
      break;
    default: assert(false);
  }
}

/**
 * Configure Transfer Rate
 *
 * \details
 * This configures the Transfer rate to `Slow`. Since currently no Decoders
 * need a speed slower than `Slow`, we assume that the Update will fail anyway
 * if this fails.
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void Update::config() {
  pushUI({.id = type::MessageID::Init});

  if (!_lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Slow))
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to set transfer rate"};
}

/**
 * Search for connected Decoder(-s)
 *
 * \details
 * This loops over all existing firmwares and pings the decoder ID. Once a
 * decoder is found, we exit with `true`. If no decoder is found, we can't
 * exactly perform an update and just abort with `false`
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void Update::search() {
  pushUI({.id = type::MessageID::SearchDecoder});

  do {
    unsigned int tries{0u};
    do {
      if (_lib.mdu_ein().ping(0, _fwIt.id())) { // Found
        return;
      }
    } while (++tries < 3u);
  } while (++_fwIt != _zsu->end());

  throw process_error{
    {.id = type::MessageID::AbortDecoderSearch, .payload = true},
    "No decoder found"};
}

/**
 * Initialize Encryption
 *
 * \details
 * Initializes the Salsa20 encryption. If this fails, we abort with `false`
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void Update::init() {
  if (!_lib.mdu_ein().zsuSalsa20Iv(_fwIt))
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to initialize the Salsa20 encryption"};
}

/**
 * Erase Decoder flash
 *
 * \details
 * Here we erase the flash of the Decoder. Since no mechanism to poll this
 * process exists, we just wait for 10s while sending `busy` as a heartbeat.
 * If this fails, we abort with `false`
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void Update::erase() {
  pushUI({.id = type::MessageID::EraseFlash});

  if (!_lib.mdu_ein().zsuErase(_fwIt))
    throw process_error{
      {.id = type::MessageID::AbortFlashErase, .payload = true},
      "Unable to start flash erase"};

  for (int i{0}; i < 100; i++) {
    if (_abort)
      throw process_error{{.id = type::MessageID::Abort, .payload = true},
                          "Process aborted"};

    pushUI({.id = type::MessageID::EraseFlash,
            .progress = static_cast<double>(i) / 100.0});
    _lib.mdu_ein().busy();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  pushUI({.id = type::MessageID::EraseFlashComplete});
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
void Update::write() {
  for (int index{0}; index < _fwIt.blockCount(); index++) {
    checkAbort();

    pushUI({.id = type::MessageID::WriteFlash,
            .progress = static_cast<double>(index + 1.0) /
                        static_cast<double>(_fwIt.blockCount())});

    if (index % 64 == 0) lifesign();

    int tries{0};
    do { // Retry up to 3 times
      if (_lib.mdu_ein().zsuUpdate(_fwIt, static_cast<uint32_t>(index))) break;
    } while (++tries < 3);

    if (tries >= 3)
      throw process_error{
        {.id = type::MessageID::AbortFlashWrite, .payload = true},
        "Max retries reached"};
  }
}

/**
 * Verify Update
 *
 * \details
 * This initiates the CRC32 verification. If this fails, we simply assume a
 * checksum error and abort with `false`
 *
 * \throws process_error  If the command failed
 * \throws klug_error     If the communication failed
 */
void Update::verify() {
  pushUI({.id = type::MessageID::Verify});

  if (!_lib.mdu_ein().zsuCrc32Start(_fwIt))
    throw process_error{{.id = type::MessageID::AbortVerify, .payload = true},
                        "Unable to start CRC verification"};
}

/**
 * End Update
 *
 * \details
 * Currently, this is the second half of the CRC32 verification. If this
 * fails, we just assume a checksum error and abort with `false`
 *
 */
void Update::end() {
  if (!_lib.mdu_ein().zsuCrc32ResultExit())
    throw process_error{{.id = type::MessageID::AbortVerify, .payload = true},
                        "Unable to finish CRC verification"};
}

/**
 * Pings selected decoder to check if any answers
 *
 */
void Update::lifesign() {
  int tries{0};
  do {
    if (_lib.mdu_ein().ping(0u, _fwIt.id())) break;
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
void Update::checkAbort() {
  if (_abort)
    throw process_error{{.id = type::MessageID::Abort, .payload = true},
                        "Process Aborted"};
}

} // namespace process::mdu_ein
