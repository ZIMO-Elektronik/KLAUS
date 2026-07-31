/**
 *  Update process
 *
 * \file    src/process/mdu_ein/update.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "include/process/mdu_ein/update.hpp"
#include <future>
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
 * Abort process
 */
void Update::abort() { _abort = true; }

/**
 * The actual update
 *
 * \details
 * This will execute components in imperative order until either completion, or
 * the first non-recoverable Error. Either way, the Update will be ended cleanly
 * and the Update device is reset.
 */
void Update::update() {
  if (!ping() || !mode() || !enter() || !config() || !search() || !init() ||
      !erase() || !write() || !verify())
    _abort = true;
  end();
  reset();
}

/**
 * Ping ULF_COM device
 *
 * \details
 * If the ping yields a result, it it pushed to the UI. Otherwise we can assume,
 * that the update will fail anyway and abort.
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::ping() {
  if (_abort) return false;

  _lib.com().ping();
  auto res = _lib.jobAwait();
  if (auto const string{std::get_if<res::String>(&res)}) {
    pushUI({.id = type::MessageID::None,
            .payload = static_cast<std::string>(*string)});
    return true;
  }

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Change mode to MDU_EIN
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::mode() {
  if (_abort) return false;

  pushUI({.id = type::MessageID::Start});
  _lib.com().mdu_ein();
  auto res = _lib.jobAwait();
  if (auto const status{std::get_if<res::Status>(&res)})
    if (*status) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Enter Decoder(-s)
 *
 * \details
 * This will perform the selected entry in \ref Update::_entryType. In case of a
 * `DCC_ZSU` entry, all decoders in the given \ref Update::_decoderIDs are
 * entered, in case it is empty, entry is done with ID and SN = 0.
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::enter() {
  if (_abort) return false;

  _err_cnt = 0uz;
  while (true) {
    switch (_entryType) {
      case type::MDUEntryType::MDU: _lib.mdu_ein().enterMDU(); break;
      case type::MDUEntryType::DCC_ZSU:
        if (_decoderIDs.empty()) _lib.mdu_ein().enterDCCZSU();
        else _lib.mdu_ein().enterDCCZSU(*_iter, 0uz, _iter == _lastIter);
        break;
      default: assert(false);
    }

    auto const res{_lib.jobAwait()};
    if (auto const status{std::get_if<res::Status>(&res)}) {
      if (*status) {
        _err_cnt = 0uz;
        if (_entryType == type::MDUEntryType::MDU ||
            (_entryType == type::MDUEntryType::DCC_ZSU &&
             (_decoderIDs.empty() || ++_iter == _decoderIDs.end())))
          return true;
      } else if (_err_cnt > 3uz) {
        pushUI({.id = type::MessageID::AbortInit, .payload = true});
        return false;
      }
    }
  }
}

/**
 * Configure Transfer Rate
 *
 * \details
 * This configures the Transfer rate to `Slow`. Since currently no Decoders need
 * a speed slower than `Slow`, we assume that the Update will fail anyway if
 * this fails.
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::config() {
  if (_abort) return false;

  pushUI({.id = type::MessageID::Init});
  _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Slow);
  auto const res{_lib.jobAwait()};
  if (auto const status{std::get_if<res::Status>(&res)}) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Search for connected Decoder(-s)
 *
 * \details
 * This loops over all existing firmwares and pings the decoder ID. Once a
 * decoder is found, we exit with `true`. If no decoder is found, we can't
 * exactly perform an update and just abort with `false`
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::search() {
  _err_cnt = 0uz;
  _updateCb({.id = type::MessageID::SearchDecoder});
  while (true) {
    if (_abort) return false;

    _lib.mdu_ein().ping(0, _fwIt.id());
    auto const res{_lib.jobAwait()};
    if (auto const status{std::get_if<res::Status>(&res)}) {
      if (*status) { // Found
        pushUI({.id = type::MessageID::FoundDecoder});
        return true;
      } else { // Not found
        if (++_fwIt == _zsu->end()) {
          pushUI({.id = type::MessageID::AbortDecoderSearch, .payload = true});
          return false;
        }
      }
    } else { // Error
      pushUI({.id = type::MessageID::AbortDecoderSearch, .payload = true});
      return false;
    }
  }
}

/**
 * Initialize Encryption
 *
 * \details
 * Initializes the Salsa20 encryption. If this fails, we abort with `false`
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::init() {
  if (_abort) return false;

  _lib.mdu_ein().zsuSalsa20Iv(_fwIt);
  auto const res{_lib.jobAwait()};
  if (auto const status{std::get_if<res::Status>(&res)}) return true;

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Erase Decoder flash
 *
 * \details
 * Here we erase the flash of the Decoder. Since no mechanism to poll this
 * process exists, we just wait for 10s while sending `busy` as a heartbeat. If
 * this fails, we abort with `false`
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::erase() {
  if (_abort) return false;

  pushUI({.id = type::MessageID::EraseFlash});
  _lib.mdu_ein().zsuErase(_fwIt);

  { // Erase
    auto const res{_lib.jobAwait()};
    if (auto const status{std::get_if<res::Status>(&res)}) {
      if (!*status) { // Error
        pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
        return false;
      }
    } else { // Error
      pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
      return false;
    }
  }

  { // Wait for erase to finish
    for (size_t i{0uz}; i < 20uz; i++) {
      if (_abort) return false;

      pushUI({.id = type::MessageID::EraseFlash,
              .progress = static_cast<double>(i) / 20.0});
      _lib.mdu_ein().busy();
      auto const res{_lib.jobAwait()};
      if (auto const status{std::get_if<res::Status>(&res)}) {
        if (!*status) { // Error
          pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
          return false;
        }
      } else { // Error
        pushUI({.id = type::MessageID::AbortFlashErase, .payload = true});
        return false;
      }

      // Wait for 0.5s
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

  pushUI({.id = type::MessageID::EraseFlashComplete});
  return true;
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
bool Update::write() {
  _err_cnt = 0uz;
  size_t index{0uz};
  while (index < _fwIt.blockCount()) {
    if (_abort) return false;

    pushUI({.id = type::MessageID::WriteFlash,
            .progress = static_cast<double>(index + 1.0) /
                        static_cast<double>(_fwIt.blockCount())});
    _lib.mdu_ein().zsuUpdate(_fwIt, static_cast<uint32_t>(index));
    auto const res{_lib.jobAwait()};
    if (auto const status{std::get_if<res::Status>(&res)}) {
      if (*status) {
        _err_cnt = 0;
        index++;
      } else _err_cnt++;
    } else {
      _err_cnt++;
    }

    if (_err_cnt > 3uz) {
      pushUI({.id = type::MessageID::AbortFlashWrite});
      return false;
    }
  }

  return true;
}

/**
 * Verify Update
 *
 * \details
 * This initiates the CRC32 verification. If this fails, we simply assume a
 * checksum error and abort with `false`
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Update::verify() {
  if (_abort) return false;

  pushUI({.id = type::MessageID::Verify});
  _lib.mdu_ein().zsuCrc32Start(_fwIt);
  auto const res{_lib.jobAwait()};
  if (auto const status{std::get_if<res::Status>(&res)}) {
    if (*status) {
      pushUI({.id = type::MessageID::Done, .payload = true});
      return true;
    }
  }

  pushUI({.id = type::MessageID::AbortVerify, .payload = true});
  return false;
}

/**
 * End Update
 *
 * \details
 * Currently, this is the second half of the CRC32 verification. If this fails,
 * we just assume a checksum error and abort with `false`
 *
 */
void Update::end() {
  _lib.mdu_ein().zsuCrc32ResultExit();
  auto const res{_lib.jobAwait()};
  if (auto const status{std::get_if<res::Status>(&res)}) {
    if (*status) return;
  }

  pushUI({.id = type::MessageID::AbortVerify, .payload = true});
}

/**
 * Reset ULF_COM device
 *
 * \details
 * Since this is after the actual update, we can ignore the result and wait for
 * completion
 *
 */
void Update::reset() {
  _lib.com().reset();
  _lib.jobAwait();
}

/**
 * Push an update to the UI (if possible)
 *
 * \param u Update
 */
void Update::pushUI(type::ProcessUpdate const& u) {
  if (_updateCb) _updateCb(u);
}

} // namespace process::mdu_ein
