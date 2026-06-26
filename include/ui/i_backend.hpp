/**
 * Backend interface
 *
 * \file    include/ui/i_backend.hpp
 * \author  Jonas Gahlert
 * \date    20.05.2026
 */

#pragma once

#include <app-window.h>
#include <slint.h>

namespace ui {

/**
 * Backend interface
 *
 * \details
 * An interface to store each backend in a singular container
 *
 * \todo
 * Unused, do we need this?
 *
 */
struct IBackend {
  virtual void connect(slint::ComponentHandle<AppWindow>) = 0;
};

} // namespace ui
