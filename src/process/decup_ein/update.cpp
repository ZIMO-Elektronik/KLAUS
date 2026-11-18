#include "update.hpp"
#include <iostream>
#include <thread>
#include "process/process_error.hpp"
#include "type/step.hpp"

namespace process::decup_ein {

Update::Update(std::filesystem::path path) {}

Update::Update(std::shared_ptr<libulf::ZPP> zpp) {}

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

    reset();
  } catch (libulf::ulf_error const& u) {
    // We catch the timeout seperately because of a bug with the KLUG
    std::cerr << u.what() << std::endl;

  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
    pushUI({.id = type::MessageID::AbortUnresponsive, .payload = true});
  }
}

void Update::mode() {}

void Update::enter() {}

libulf::ZSU::Firmware Update::search() {
  libulf::ZSU::Firmware returnValue;
  return returnValue;
}

void Update::setup(libulf::ZSU::Firmware& fw) {}

void Update::write(libulf::ZSU::Firmware& fw) {}

void Update::checkAbort() {}

} // namespace process::decup_ein
