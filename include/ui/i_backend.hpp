/**
 * Backend interface
 *
 * \file    include/ui/i_backend.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <slint.h>

namespace ui {

/**
 * Backend interface
 *
 */
struct IBackend {
  virtual void connect(slint::ComponentHandle<AppWindow>) = 0;
};

} // namespace ui
