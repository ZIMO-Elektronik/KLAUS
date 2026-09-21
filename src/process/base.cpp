/**
 * Copyright (C) 2026 [ZIMO Elektronik]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://gnu.org>.
 *
 *
 *
 *
 *
 * Base process
 *
 * \file    process/base.cpp
 * \author  Jonas Gahlert
 * \date    21.09.2026
 */

#include "process/base.hpp"
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
  if (_lib.init() != libulf::Error::ok ||
      _lib.open(0x1FC9u, 0x81C1u) != libulf::Error::ok)
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
  if (!_process.valid() ||
      _process.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    return true;

  return false;
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
 * \throws ulf_error     If the communication failed
 */
void Base::ping() {
  pushUI(
    {.id = type::MessageID::None, .payload = std::move(_lib.com().ping())});
}

/**
 * Reset Device
 *
 * \throws ulf_error     If the communication failed
 */
void Base::reset() {
  _lib.com().reset();

  if (!_abort) pushUI({.id = type::MessageID::Done, .payload = true});
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
