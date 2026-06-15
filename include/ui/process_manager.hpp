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

struct ProcessManager : IBackend {

  virtual void connect(slint::ComponentHandle<AppWindow>) override;

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
