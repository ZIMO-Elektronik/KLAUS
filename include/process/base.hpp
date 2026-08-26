/**
 * Base process
 *
 * \file    include/process/base.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <future>
#include <libklug/libklug.hpp>
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
  bool ping();
  bool reset();

  void pushUI(type::ProcessUpdate const& u);

  libklug::LibKLUG _lib; ///< Libklug handle

  bool _done{false};  ///< Process done
  bool _abort{false}; /// Abort process

  std::future<void> _process; ///< Process future

  std::function<void(type::ProcessUpdate)> _updateCb{}; ///< GUI update callback
};

} // namespace process
