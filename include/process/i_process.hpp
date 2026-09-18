/**
 * Process interface
 *
 * \file    include/process/i_process.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <functional>
#include "include/type/step.hpp"

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
