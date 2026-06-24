/**
 * ProcessManager
 *
 * \file    include/ui/process_manager.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

#include <app-window.h>
#include <slint.h>
#include <concepts>
#include <mutex>
#include "app-window.h"
#include "include/process/i_process.hpp"
#include "include/type/step.hpp"
#include "include/ui/helper/progress_tracker.hpp"
#include "include/ui/i_backend.hpp"
#include "slint.h"

namespace ui {

/**
 * Manages the interface between a (running) backend process and the UI.
 *
 * \details
 * As an extra, an instance of this is also used to determine if a process is
 * running and if a process can be started.
 *
 * Using \ref ProcessManager::emplace, a new process can be created. This will
 * only have an effect if the current process is done or if none exists. \ref
 * ProcessManager::execute will execute the emplaced process, while \ref
 * ProcessManager::abort will abort it.
 *
 * To connect it to the UI, use the \ref ProcessManager::connect method
 *
 */
struct ProcessManager : IBackend {

  virtual void connect(slint::ComponentHandle<AppWindow>) override;

  /**
   * Emplace a process
   *
   * \note
   * This only has an effect if no process exists or if the existing process is
   * done. This will replace an existing process
   *
   * \tparam T Process type (explicit)
   * \tparam Args Process CTor args
   *
   * \param args Args
   *
   * \return true   Process emplaced
   * \return false  Unable to emplace
   */
  template<typename T, typename... Args>
  requires std::derived_from<T, process::IProcess> &&
           std::constructible_from<T, Args...>
  bool emplace(Args&&... args) {
    std::unique_lock<std::mutex> lock(_mut_process);

    // Check if a process exists
    if (_process) {
      // Check if existing process is done
      if (!_process->done()) return false;
    }

    _process = std::make_unique<T>(std::forward<Args>(args)...);
    return true;
  }

  bool busy();

  bool execute();
  void abort();
  void done();

private:
  void updateProgress(double progress);
  void updateText(type::MessageID id, bool force);

  slint::ComponentWeakHandle<AppWindow> _weakUi{}; ///< Weak UI handle

  std::mutex _mut_process{};                     ///< Process mutex
  std::unique_ptr<process::IProcess> _process{}; ///< Running process

  type::MessageID _lastId{}; ///< Last messageId

  helper::ProgressTracker _tracker{}; ///< Progress time tracker
};

} // namespace ui
