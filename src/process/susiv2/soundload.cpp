#include "include/process/susiv2/soundload.hpp"
#include <iostream>

namespace process::susiv2 {

SoundLoad::SoundLoad(std::filesystem::path path) : _zpp{path} { connect(); }
SoundLoad::~SoundLoad() { disconnect(); }

void SoundLoad::execute() { modeAction(); }

void SoundLoad::abort() { _abort = true; }

void SoundLoad::setup(std::shared_ptr<SoundLoad> thiz) {
  _lib.registerCb(thiz, &SoundLoad::handle_result);
}

void SoundLoad::onUpdateProgress(std::function<void(double)> cb) {
  _progressCb = cb;
}

void SoundLoad::onUpdateStep(std::function<void(type::SoundLoadStep)> cb) {
  _stepCb = cb;
}

void SoundLoad::handle_result(res::Result r) {
  if (_abort) return resetAction();
  std::invoke(_state, this, r);
}

void SoundLoad::modeAction() {
  _stepCb(type::SoundLoadStep::Start);
  _state = &SoundLoad::modeResult;
  _lib.com().susiv2();
}

void SoundLoad::modeResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Entered SUSIV2" << std::endl;
      return featuresAction();
    }
  }

  std::cout << "Unable to enter SUSIV2" << std::endl;
  return resetAction();
}

void SoundLoad::featuresAction() {
  _stepCb(type::SoundLoadStep::Init);
  _state = &SoundLoad::featuresResult;
  _lib.susiv2().features();
}

void SoundLoad::featuresResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Requested features" << std::endl;
      return eraseAction();
    }
  }

  std::cout << "Unable to request features" << std::endl;
  return resetAction();
}

void SoundLoad::eraseAction() {
  _stepCb(type::SoundLoadStep::Erase);
  _state = &SoundLoad::eraseResult;
  _lib.susiv2().zppErase();
}

void SoundLoad::eraseResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Erased flash" << std::endl;
      return loadAction();
    }
  }

  std::cout << "Unable to erase flash" << std::endl;
  return resetAction();
}

void SoundLoad::loadAction() {
  _stepCb(type::SoundLoadStep::Load);
  _state = &SoundLoad::loadResult;
  _lib.susiv2().zppWrite(_zpp, _index);
}

void SoundLoad::loadResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      // Block written
      _progressCb(static_cast<double>(_index + 1.0) /
                  static_cast<double>(_zpp.blocks()));

      if (++_index >= _zpp.blocks()) { return endAction(); }
      return loadAction();
    } else {
      // Block rejected
      if (_err_cnt++ < 3) {
        // Retry
        return loadAction();
      }

      // Too many errors
    }
  }

  std::cout << "Unable to write flash" << std::endl;
  return resetAction();
}

void SoundLoad::endAction() {
  _stepCb(type::SoundLoadStep::Cleanup);
  _state = &SoundLoad::endResult;
  _lib.susiv2().exit(true, true);
}

void SoundLoad::endResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Exited" << std::endl;
      return resetAction();
    }
  }

  std::cout << "Unable to exit" << std::endl;
  return resetAction();
}

void SoundLoad::resetAction() {
  _lib.com().reset();
  _state = &SoundLoad::resetResult;
}

void SoundLoad::resetResult(res::Result const& r) {
  if (std::holds_alternative<res::Status>(r))
    std::cout << "Reset success" << std::endl;
  else std::cout << "Reset NOT successful" << std::endl;

  _stepCb(type::SoundLoadStep::Done);
  return;
}

} // namespace process::susiv2
