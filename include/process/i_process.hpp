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

  virtual bool done() = 0;

  virtual void onUpdate(std::function<void(type::ProcessUpdate)>) = 0;
};

} // namespace process
