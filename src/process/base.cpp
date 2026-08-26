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
  if (_lib.init() != err::Error::ok ||
      _lib.open(0x1FC9u, 0x81C1u) != err::Error::ok)
    return false;
  return true;
}

/**
 * Disconnect device
 *
 */
void Base::disconnect() { _lib.close(); }

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
 * Mark process to abort
 *
 */
void Base::abort() { _abort = true; }

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

/**
 * Ping device
 *
 * \details
 * If the ping yields a result, it it pushed to the UI. Otherwise we can assume,
 * that the any further work will fail anyway and abort.
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Base::ping() {
  if (auto const res{_lib.com().ping()}) {
    pushUI({.id = type::MessageID::None, .payload = std::move(*res)});
    return true;
  }

  pushUI({.id = type::MessageID::AbortInit, .payload = true});
  return false;
}

/**
 * Reset Device
 *
 * \return true   Continue
 * \return false  Abort
 */
bool Base::reset() {
  if (!_abort) pushUI({.id = type::MessageID::Done, .payload = true});

  if (auto const res{_lib.com().reset()})
    if (*res) return true;

  return false;
}

/**
 * Push an update to the UI (if possible)
 *
 * \param u Update
 */
void Base::pushUI(type::ProcessUpdate const& u) {
  if (_updateCb) _updateCb(u);
}

} // namespace process
