#include "include/process/base.hpp"
#include <iostream>

namespace process {

Base::Base() {}

bool Base::connect() {
  auto rc{_lib.init()};
  if (rc) return false;

  std::cout << "Inited" << std::endl;

  rc = _lib.open(0x1FC9u, 0x81C1u);
  if (rc) return false;

  std::cout << "Opened" << std::endl;

  rc = _lib.config();
  if (rc) return false;

  std::cout << "Configured" << std::endl;

  rc = _lib.claim();
  if (rc) return false;

  std::cout << "Claimed" << std::endl;

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
