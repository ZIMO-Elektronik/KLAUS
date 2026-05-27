#include "include/process/base.hpp"

namespace process {

Base::Base() { bool donezo = false; }

bool Base::connect() {
  auto rc{_lib.init()};
  if (rc) return false;

  rc = _lib.open(0x1FC9u, 0x81C1u);
  if (rc) return false;

  rc = _lib.config();
  if (rc) return false;

  rc = _lib.claim();
  if (rc) return false;

  return true;
}

void Base::disconnect() {
  _lib.release();
  _lib.close();
}

} // namespace process
