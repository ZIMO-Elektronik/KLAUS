/**
 * Cv process interface
 *
 * \file    include/process/i_cv_process.hpp
 * \author  Jonas Gahlert
 * \date    26.05.2026
 */

#pragma once

#include <functional>
#include "i_process.hpp"
#include "include/type/step.hpp"

namespace process {

/**
 * Cv process interface
 *
 */
struct ICvProcess : public IProcess {
  virtual void onUpdateStep(std::function<void(type::CvStep)>) = 0;
};

} // namespace process
