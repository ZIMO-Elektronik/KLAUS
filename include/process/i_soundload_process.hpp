/**
 * Soundload process interface
 *
 * \file    include/process/i_soundload_process.hpp
 * \author  Jonas Gahlert
 * \date    26.05.2026
 */

#pragma once

#include <functional>
#include "i_process.hpp"
#include "include/type/step.hpp"

namespace process {

/**
 * Soundload process interface
 *
 */
struct ISoundLoadProcess : public IProcess {
  virtual void onUpdateStep(std::function<void(type::SoundLoadStep)>) = 0;
};

} // namespace process
