/**
 * Process interface
 *
 * \file    include/process/i_process.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

namespace process {

/**
 * Process interface
 *
 */
struct IProcess {
  virtual void execute() = 0;
  virtual void abort() = 0;

  virtual double progress() = 0;
};

} // namespace process
