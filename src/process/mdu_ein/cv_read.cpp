#include "include/process/mdu_ein/cv_read.hpp"

namespace process::mdu_ein {

CvRead::CvRead() {
  _lib = libklug_create();
  libklug_init(_lib);

  libklug_open(_lib, 0x1FC9u, 0x81C1u);
  libklug_config(_lib);
  libklug_claim(_lib);
}

CvRead::~CvRead() {
  libklug_release(_lib);
  libklug_close(_lib);

  libklug_destroy(_lib);
}

void CvRead::execute() {
  libklug_register_cb(_lib, std::function<void(result const)>{})

    return modeAction();
}

void CvRead::abort() { _abort = true; }

void CvRead::handle(result const r) {
  if (!_abort) { return std::invoke(_state, this, r); }

  // Code to abort process
}

void CvRead::modeAction() { _state = CvRead::modeResult; }

void CvRead::modeResult(result const r) {}

} // namespace process::mdu_ein
