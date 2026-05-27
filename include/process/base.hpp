/**
 * Base process
 *
 * \file    include/process/base.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <libklug/libklug.hpp>
#include "i_process.hpp"

namespace process {

/**
 * Base process
 *
 */
struct Base {
  Base();            // Create handle
  ~Base() = default; // Destroy handle

  bool connect();
  void disconnect();

protected:
  libklug::LibKLUG _lib;
};

} // namespace process
