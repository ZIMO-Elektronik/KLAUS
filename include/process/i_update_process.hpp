/**
 * Update process interface
 *
 * \file    include/process/i_Update_process.hpp
 * \author  Jonas Gahlert
 * \date    26.05.2026
 */

#pragma once

#include <functional>
#include "i_process.hpp"
#include "include/type/step.hpp"

namespace process {

/**
 * Update process interface
 *
 */
struct IUpdateProcess : IProcess {
  virtual void onUpdateStep(std::function<void(type::UpdateStep)>) = 0;
};

} // namespace process
