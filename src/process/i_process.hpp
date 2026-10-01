/**
 * Copyright (C) 2026 ZIMO Elektronik
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
 * Process interface
 *
 * \file    process/i_process.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <functional>
#include "type/step.hpp"

namespace process {

/**
 * Process interface
 *
 */
struct IProcess {
  virtual ~IProcess() = default;

  /**
   * Execute the process
   *
   * \details
   * This starts the process, if it is able to start.
   *
   * \note
   * If the underlying process is unable to start, the \ref IProcess::done
   * checker may not reflect this.
   *
   * \return true   Started
   * \return false  Unable to start
   */
  virtual bool execute() = 0;

  /**
   * Abort the process
   *
   * \details
   * This aborts the process, once it is able to handle an abort. This may take
   * some time, if the process is e.g. deleting data in ZUSI, since the command
   * is currently blocking until finish.
   *
   */
  virtual void abort() = 0;

  /**
   * Is process done?
   *
   * \details
   * This checks, if the process is done with it's job and can be safely deleted
   *
   * \note
   * Regarding the 'safe to delete'-bit.. It signals `done`, then performs
   * cleanup, so it may be saver to wait another 10ms to be sure.
   *
   * \return true   Done
   * \return false  Busy
   */
  virtual bool done() = 0;

  /**
   * Update state
   *
   * \details
   * This registers a callable and pushes updates through it. These updates may
   * happen at any time.
   *
   * \warning
   * Currently, there is no deregister.. So the owner of the callable may not go
   * out of scope.a
   *
   * \param function Update callable (e.g. a lambda)
   *
   */
  virtual void onUpdate(std::function<void(type::ProcessUpdate)>) = 0;
};

} // namespace process
