#include "include/process/mdu_ein/update.hpp"

#include <iostream>
#include <thread>

namespace process::mdu_ein {

Update::Update(std::filesystem::path path) : Base{}, _zsu{path} { connect(); }
Update::~Update() { disconnect(); }

void Update::execute() { modeAction(); }

void Update::abort() { _abort = true; }

void Update::setup(std::shared_ptr<Update> thiz) {
  _lib.registerCb(thiz, &Update::handle_result);
}

void Update::onUpdateProgress(std::function<void(double)> cb) {
  _updateProgress = cb;
}

void Update::onUpdateStep(std::function<void(type::UpdateStep)> cb) {
  _updateStep = cb;
}

void Update::handle_result(res::Result r) {
  if (_abort) resetAction();
  std::invoke(_state, this, r);
}

void Update::modeAction() {
  _updateStep(type::UpdateStep::Start);
  _lib.com().mdu_ein();
  _state = &Update::modeResult;
}

void Update::modeResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Mode MDU_EIN" << std::endl;
    enterAction();
    return;
  }

  std::cout << "Unable to change mode" << std::endl;
  resetAction();
}

void Update::enterAction() {
  _lib.mdu_ein().enterMDU();
  _state = &Update::enterResult;
}

void Update::enterResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Entered via MDU" << std::endl;
    configAction();
    return;
  }

  std::cout << "Unable to enter" << std::endl;
  resetAction();
}

void Update::configAction() {
  _updateStep(type::UpdateStep::Init);
  _lib.mdu_ein().configTransferRate(libklug::mdu::Speed::Slow);
  _state = &Update::configResult;
}

void Update::configResult(res::Result const r) {
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

void Update::searchAction() {
  _updateStep(type::UpdateStep::Search);
  _lib.mdu_ein().ping(0, _fwIt.id());
  _state = &Update::searchResult;
}

void Update::searchResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Ping";
    if (std::get<res::Status>(r)) {
      std::cout << "Successful at Id 0x" << std::hex << _fwIt.id() << std::dec
                << std::endl;
      initAction();
      return;

    } else {
      std::cout << "Unsuccessful at Id 0x" << std::hex << _fwIt.id() << std::dec
                << std::endl;
      ++_fwIt;
      if (_fwIt == _fwItEnd) {
        std::cout << "No decoder found" << std::endl;
        resetAction();
        return;
      }

      searchAction();
      return;
    }
  }

  std::cout << "Unable to ping" << std::endl;
  resetAction();
}

void Update::initAction() {
  _lib.mdu_ein().zsuSalsa20Iv(_fwIt);
  _state = &Update::initResult;
}

void Update::initResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Salsa20 initialized" << std::endl;
      eraseAction();
      return;
    }
  }
  std::cout << "Unable to init Salsa20" << std::endl;
  resetAction();
  return;
}

void Update::eraseAction() {
  _updateStep(type::UpdateStep::Erase);
  _lib.mdu_ein().zsuErase(_fwIt);
  _state = &Update::eraseResult;
}

void Update::eraseResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Erasing" << std::endl;
      waitAction();
      return;
    }
  }
  std::cout << "Unable to Erase" << std::endl;
  resetAction();
  return;
}

void Update::waitAction() {
  _updateProgress(static_cast<double>(_index) / 20.0);
  _lib.mdu_ein().busy();
  _state = &Update::waitResult;
}

void Update::waitResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (_index++ >= 20) {
      std::cout << "Finished erasing" << std::endl;
      _index = 0;
      updateAction();
      return;
    }
    std::cout << "Still erasing" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    waitAction();
    return;
  }

  std::cout << "Error while waiting for erasing" << std::endl;
  resetAction();
  return;
}

void Update::updateAction() {
  _updateStep(type::UpdateStep::Update);
  _lib.mdu_ein().zsuUpdate(_fwIt, _index);
  _state = &Update::updateResult;
}

void Update::updateResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      _updateProgress(static_cast<double>(_index + 1.0) /
                      static_cast<double>(_fwIt.blocks()));
      if (_index % 256 == 0) {
        std::cout << "Written " << _index << " Blocks" << std::endl;
      }

      if (++_index >= _fwIt.blocks()) {
        std::cout << "Written " << _fwIt.blocks() << " Blocks" << std::endl;
        verifyAction();
        return;
      }
      updateAction();
      return;
    }
  }
  std::cout << "Error while updating" << std::endl;
  resetAction();
}

void Update::verifyAction() {
  _updateStep(type::UpdateStep::Verify);
  _lib.mdu_ein().zsuCrc32Start(_fwIt);
  _state = &Update::verifyResult;
}

void Update::verifyResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "Started CRC32 verification" << std::endl;
      endAction();
      return;
    }
  }
  std::cout << "Unable to start verify" << std::endl;
  resetAction();
}

void Update::endAction() {
  _updateStep(type::UpdateStep::Cleanup);
  _lib.mdu_ein().zsuCrc32ResultExit();
  _state = &Update::endResult;
}

void Update::endResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    if (std::get<res::Status>(r)) {
      std::cout << "CRC Success" << std::endl;
    } else {
      std::cout << "CRC Error" << std::endl;
    }
    resetAction();
    return;
  }
  std::cout << "Unable to verify" << std::endl;
  resetAction();
}

void Update::resetAction() {
  _lib.com().reset();
  _state = &Update::resetResult;
}

void Update::resetResult(res::Result const r) {
  if (std::holds_alternative<res::Status>(r)) {
    std::cout << "Reset success" << std::endl;

    return;
  }
  std::cout << "Reset NOT successful" << std::endl;
  _updateStep(type::UpdateStep::Done);
  return;
}

} // namespace process::mdu_ein
