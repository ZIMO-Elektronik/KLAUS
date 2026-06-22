#include "include/process/base.hpp"
#include <cassert>
#include <iostream>

namespace process {

Base::Base() {}

bool Base::connect() {

  for (int i{0}; i < 4; i++) {
    int rc{0};
    switch (i) {
      case 0: rc = _lib.init(); break;
      case 1: rc = _lib.open(0x1FC9u, 0x81C1u); break;
      case 2: rc = _lib.config(); break;
      case 3: rc = _lib.claim(); break;
      default: assert(false);
    }
    if (rc) return false;
  }

  return true;
}

void Base::disconnect() {
  _lib.release();
  _lib.close();
}

bool Base::done() { return _done; }

void Base::onUpdate(std::function<void(type::ProcessUpdate)> cb) {
  _updateCb = cb;
}

} // namespace process
