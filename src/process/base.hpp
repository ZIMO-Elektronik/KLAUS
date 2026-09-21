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
 * \file    process/base.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <future>
#include <ulf/cpp/libulf.hpp>
#include "i_process.hpp"

namespace process {

/**
 * Base process
 *
 * \details
 * Basic functions like connecting and disconnecting the USB device and updating
 * the UI are placed here.
 *
 * Use \ref Base::connect and \ref Base::disconnect to setup or teardown the USB
 * connection.
 *
 * From the UI, \ref Base::done provides a poll interface to check if the
 * process has finished. This does NOT mean that it finished with success, it is
 * just finished.
 *
 * To update the UI (or anything else) with the state of the process, a callable
 * can be registered via \ref onUpdate. Updates may come at any time.
 *
 * \warning
 * Any callback inserted into \ref Base::onUpdate MUST be UI-thread safe, since
 * any update form here WILL be coming from another thread.
 *
 */
struct Base : IProcess {
  Base();
  virtual ~Base() override = default;

  bool connect();
  void disconnect();

  virtual bool done() override;
  virtual void abort() override;

  virtual void onUpdate(std::function<void(type::ProcessUpdate)>) override;

protected:
  void ping();
  void reset();

  void pushUI(type::ProcessUpdate const& u);

  libulf::LibULF _lib; ///< Libulf handle

  bool _abort{false}; /// Abort process

  std::future<void> _process; ///< Process future

  std::function<void(type::ProcessUpdate)> _updateCb{}; ///< GUI update callback
};

} // namespace process
