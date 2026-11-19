#include "update.hpp"
#include <iostream>
#include <thread>
#include "process/process_error.hpp"
#include "type/step.hpp"

namespace process::decup_ein {

Update::Update(std::filesystem::path path)
  : _zsu{std::make_shared<libulf::ZSU>(path)} {}

Update::Update(std::shared_ptr<libulf::ZSU> zsu) : _zsu{zsu} {}

Update::~Update() { disconnect(); }

bool Update::execute() {
  if (_zsu == nullptr || !_zsu->valid() || !connect()) { return false; }

  _process = std::async([this]() { return this->update(); });
  return true;
}

void Update::update() {
  try {
    try {
      mode();
      enter();
      auto fw{search()};
      setup(fw);
      write(fw);
    } catch (process_error const& e) {
      std::cerr << e.what() << std::endl;
      _abort = true;
      pushUI(static_cast<type::ProcessUpdate>(e));
    }
    end();
    reset();
  } catch (libulf::ulf_error const& u) {
    // We catch the timeout seperately because of a bug with the KLUG
    std::cerr << u.what() << std::endl;

    if (!_abort) pushUI({.id = type::MessageID::Done, .payload = true});

  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
    pushUI({.id = type::MessageID::AbortUnresponsive, .payload = true});
  }
}

void Update::mode() {
  checkAbort();

  pushUI({.id = type::MessageID::Start});

  if (!_lib.com().decup_ein() || (_lib.setRTS(true) != libulf::Error::ok) ||
      (_lib.setDTR(true) != libulf::Error::ok))
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to change mode"};
}

void Update::enter() {
  checkAbort();

  pushUI({.id = type::MessageID::Init});

  if (!_lib.decup_ein().enterZSU())
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to reset decoder"};
}

libulf::ZSU::Firmware Update::search() {
  checkAbort();

  pushUI({.id = type::MessageID::SearchDecoder});

  auto firmwares{_zsu->firmwares()};
  for (auto fw : firmwares) {
    if (_lib.decup_ein().zsuDecoderId(fw.id())) return fw;
  }

  throw process_error{
    {.id = type::MessageID::AbortDecoderSearch, .payload = true},
    "No suitable decoder found"};
  std::unreachable();
}

void Update::setup(libulf::ZSU::Firmware& fw) {
  checkAbort();

  pushUI({.id = type::MessageID::Init});

  if (!_lib.decup_ein().zsuPageCount(fw) ||
      !_lib.decup_ein().zsuSecurityBytes())
    throw process_error{{.id = type::MessageID::AbortInit, .payload = true},
                        "Unable to setup"};
}

void Update::write(libulf::ZSU::Firmware& fw) {
  for (unsigned int index{0u}; index < fw.blockCount(); index++) {
    checkAbort();

    pushUI({.id = type::MessageID::WriteFlash,
            .progress = static_cast<double>(index + 1.0) /
                        static_cast<double>(fw.blockCount())});

    int tries{0};
    do { // Retry up to 3 times
      if (_lib.decup_ein().zsuBlocks(fw, static_cast<uint32_t>(index))) break;
    } while (++tries < 3);

    if (tries >= 3)
      throw process_error{
        {.id = type::MessageID::AbortFlashWrite, .payload = true},
        "Max retries reached"};
  }
}

void Update::end() {
  pushUI({.id = type::MessageID::Cleanup});

  _lib.setRTS(false);
  _lib.setDTR(false);
}

void Update::checkAbort() {
  if (_abort)
    throw process_error{{.id = type::MessageID::Abort, .payload = true},
                        "Process Aborted"};
}

} // namespace process::decup_ein
