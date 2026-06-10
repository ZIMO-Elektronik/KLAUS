#include "include/process/mdu_ein/soundload.hpp"
#include <iostream>
#include <thread>

namespace process::mdu_ein {

SoundLoad::SoundLoad(std::filesystem::path path) : _zpp{path} {
  connect();
  _lib.registerCb<[] {}>(
    [this](res::Result const result) { this->handle_result(result); });
}

SoundLoad::~SoundLoad() {
  _lib.deregisterCb();
  disconnect();
}

void SoundLoad::execute() { return modeAction(); }

void SoundLoad::abort() { _abort = true; }

void SoundLoad::onUpdateProgress(std::function<void(double)> cb) {
  _updateProgress = cb;
}

void SoundLoad::onUpdateStep(std::function<void(type::SoundLoadStep)> cb) {
  _updateStep = cb;
}

void SoundLoad::handle_result(res::Result r) {
  if (_abort) resetAction();
  std::invoke(_state, this, r);
}

void SoundLoad::modeAction() {
  _updateStep(type::SoundLoadStep::Start);
  _lib.com().mdu_ein();
  _state = &SoundLoad::modeResult;
}

void SoundLoad::modeResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Mode MDU_EIN" << std::endl;
    enterAction();
    return;
  }

  std::cout << "Unable to change mode" << std::endl;
  resetAction();
}

void SoundLoad::enterAction() {
  _lib.mdu_ein().enterDCCZPP();
  _state = &SoundLoad::enterResult;
}

void SoundLoad::enterResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Entered via MDU" << std::endl;
    configAction();
    return;
  }

  std::cout << "Unable to enter" << std::endl;
  resetAction();
}

void SoundLoad::configAction() {
  _updateStep(type::SoundLoadStep::Init);
  _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Slow);
  _state = &SoundLoad::configResult;
}

void SoundLoad::configResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Data rate set to slow" << std::endl;
      searchAction();
      return;
    }
  }
  std::cout << "Unable to set data rate" << std::endl;
  resetAction();
}

void SoundLoad::searchAction() {
  _updateStep(type::SoundLoadStep::Search);
  _lib.mdu_ein().ping(0uz, 0uz);
  _state = &SoundLoad::searchResult;
}

void SoundLoad::searchResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Ping";
    if (std::get<res::Status>(r)) {
      _err_cnt = 0;
      std::cout << "Successful at Id 0x" << std::hex << 0uz << std::dec
                << std::endl;
      initAction();
      return;

    } else {
      std::cout << "Unsuccessful at Id 0x" << std::hex << 0uz << std::dec
                << std::endl;

      if (_err_cnt++ < 3) {
        // 3 retries
        return searchAction();
      }
    }
  }

  std::cout << "Unable to ping" << std::endl;
  resetAction();
}

void SoundLoad::initAction() {
  _updateStep(type::SoundLoadStep::Init);
  _lib.mdu_ein().zppValidQuery(_zpp);
  _state = &SoundLoad::initResult;
}

void SoundLoad::initResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "ZPP valid" << std::endl;
      eraseAction();
      return;
    }
  }
  std::cout << "Unable to check if ZPP is valid" << std::endl;
  resetAction();
  return;
}

void SoundLoad::eraseAction() {
  _updateStep(type::SoundLoadStep::Erase);
  _lib.mdu_ein().zppErase(_zpp);
  _state = &SoundLoad::eraseResult;
}

void SoundLoad::eraseResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Erasing..." << std::endl;
      return waitAction();
    }
  }
  std::cout << "Unable to erase flash" << std::endl;
  resetAction();
  return;
}

void SoundLoad::waitAction() {
  _updateProgress(static_cast<double>(_index) / 200.0);
  _lib.mdu_ein().busy();
  _state = &SoundLoad::waitResult;
}

void SoundLoad::waitResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      _index = 0;
      std::cout << "Erased" << std::endl;
      return updateAction();
    }
    _index++;
    std::cout << "Still erasing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return waitAction();
  }
  std::cout << "Unable to wait for erase complete" << std::endl;
  return resetAction();
}

void SoundLoad::updateAction() {
  _updateStep(type::SoundLoadStep::Load);
  _lib.mdu_ein().zppUpdate(_zpp, _index);
  _state = &SoundLoad::updateResult;
}

void SoundLoad::updateResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      // Block written
      _err_cnt = 0;
      _updateProgress(static_cast<double>(_index + 1.0) /
                      static_cast<double>(_zpp.blocks()));
      if (++_index >= _zpp.blocks()) { return endAction(); }
      return updateAction();
    }
    // Block rejected
    if (_err_cnt++ < 3) { return updateAction(); }
  }
  std::cout << "Unable to write flash" << std::endl;
  return resetAction();
}

void SoundLoad::endAction() {
  _updateStep(type::SoundLoadStep::Cleanup);
  _lib.mdu_ein().zppUpdateEnd(_zpp);
  _state = &SoundLoad::endResult;
}

void SoundLoad::endResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) { return exitAction(); }
  }
  std::cout << "Unable to perform updateEnd" << std::endl;
  return resetAction();
}

void SoundLoad::exitAction() {
  _lib.mdu_ein().zppExitReset();
  _state = &SoundLoad::exitResult;
}

void SoundLoad::exitResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) { return resetAction(); }
  }
  std::cout << "Unable to perform exitReset" << std::endl;
  return resetAction();
}

void SoundLoad::resetAction() {
  _lib.com().reset();
  _state = &SoundLoad::resetResult;
}
void SoundLoad::resetResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r))
    std::cout << "Reset success" << std::endl;
  else std::cout << "Reset NOT successful" << std::endl;

  _updateStep(type::SoundLoadStep::Done);
  return;
}

} // namespace process::mdu_ein
