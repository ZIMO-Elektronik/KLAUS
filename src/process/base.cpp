#include "include/process/base.hpp"
#include <cassert>
#include <iostream>

namespace process {

/**
 * CTor
 *
 */
Base::Base() {}

/**
 * Connect device
 *
 * \note
 * This will connect the first device matching the PID:VID filter.
 *
 * \return true   Found and connected
 * \return false  Not found or not connected
 */
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

/**
 * Disconnect device
 *
 */
void Base::disconnect() {
  _lib.release();
  _lib.close();
}

/**
 * Check if the process is done
 *
 * \return true   Done
 * \return false  Busy
 */
bool Base::done() {
  if (_done) return true;

  if (_process.valid() &&
      _process.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    _done = true;

  return _done;
}

/**
 * Interface to register callable for UI updates
 *
 * \warning
 * Anything registered here MUST be safe to use from another thread
 *
 * \param cb Callable
 */
void Base::onUpdate(std::function<void(type::ProcessUpdate)> cb) {
  _updateCb = cb;
}

} // namespace process
