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
  virtual bool execute() = 0;
  virtual void abort() = 0;

  /// Flag should be set on finish
  /// During emplace this is checked and if false, the current process is
  /// destroyed and a new one is created
  // virtual bool busy() = 0;

  virtual void onUpdateProgress(std::function<void(double)>) = 0;
  virtual void onUpdate(std::function<void(type::ProcessUpdate)>) = 0;
};

} // namespace process
