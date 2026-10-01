/**
 * Copyright (C) 2026 ZIMO Elektronik
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
 * Main app backend
 *
 * \file    ui/app_backend.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

#include <slint.h>
#include "i_backend.hpp"
#include "process_manager.hpp"
#include "soundload_backend.hpp"
#include "update_backend.hpp"

namespace ui {

/**
 * Main app backend
 *
 * \details
 * Sortof works like a connection dispatcher and initial-state setup.a
 *
 * It's main responsibility is to re-connect the UI when the page changes to
 * avoid having to create each callback for every occurrence in a page
 *
 */
struct AppBackend : IBackend {
  AppBackend();
  virtual ~AppBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow> ui);

  void change_page(AppPage index);

private:
  slint::ComponentWeakHandle<AppWindow> _weakUi{}; ///< WeakHandle to UI

  std::shared_ptr<ProcessManager> _pManager{}; ///< Process Manager

  std::unique_ptr<UpdateBackend> _updateBackend{}; ///< Update page backend
  std::unique_ptr<SoundLoadBackend>
    _soundLoadBackend{}; ///< SoundLoad page backend
};

} // namespace ui
