/**
 * Copyright (C) 2026 [ZIMO Elektronik]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://gnu.org>.
 *
 *
 *
 *
 *
 * Backend interface
 *
 * \file    ui/i_backend.hpp
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
