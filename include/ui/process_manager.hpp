#pragma once

#include <app-window.h>
#include <slint.h>
#include <concepts>
#include <mutex>
#include "include/process/i_process.hpp"

namespace ui {

struct ProcessManager {

  virtual void connect(slint::ComponentHandle<AppWindow>);

  template<typename T, typename... Args>
  requires std::derived_from<T, process::IProcess> &&
           std::constructible_from<T, Args...>
  bool emplace(Args&&... args) {
    std::unique_lock<std::mutex> lock(_mut_process);

    // Check if a process exists
    if (_process) { return false; }

    _process = std::make_unique<T>(std::forward<Args>(args)...);
    return true;
  }

  bool execute();
  void abort();

private:
  std::mutex _mut_process{};                     ///< Process mutex
  std::unique_ptr<process::IProcess> _process{}; ///< Running process
};

} // namespace ui
