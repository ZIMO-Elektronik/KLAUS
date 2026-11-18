#include "soundload.hpp"

namespace process::decup_ein {

SoundLoad::SoundLoad(std::filesystem::path path) {}

SoundLoad::SoundLoad(std::shared_ptr<libulf::ZPP> zpp) {}

bool SoundLoad::execute() {
  if (_zpp == nullptr || !_zpp->valid() || !connect()) { return false; }

  _process = std::async([this]() { return this->load(); });
  return true;
}

void SoundLoad::load() {}

void SoundLoad::mode() {}

void SoundLoad::enter() {}

void SoundLoad::setup() {}

void SoundLoad::erase() {}

void SoundLoad::write() {}

void SoundLoad::writeCVs() {}

void SoundLoad::checkAbort() {}

} // namespace process::decup_ein
