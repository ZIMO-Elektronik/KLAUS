/**
 * Base process
 *
 * \file    include/process/base.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include "i_process.hpp"

namespace process {

/**
 * Base process
 *
 */
struct Base : public IProcess {
  Base();  // Create handle
  ~Base(); // Destroy handle

protected:
  // libklug handle
};

} // namespace process
