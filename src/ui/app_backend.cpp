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
 * Main App Backend
 *
 * \file    ui/app_backend.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "ui/app_backend.hpp"

// To safely convert the version string
#define STRINGIFY(x) TO_STRING(x)
#define TO_STRING(x) #x

namespace ui {

/**
 * CTor
 *
 */
AppBackend::AppBackend()
  : _pManager{std::make_shared<ProcessManager>()},
    _updateBackend{std::make_unique<UpdateBackend>(_pManager)},
    _soundLoadBackend{std::make_unique<SoundLoadBackend>(_pManager)} {}

/**
 * Connect to UI
 *
 * \param ui UI handle
 */
void AppBackend::connect(slint::ComponentHandle<AppWindow> ui) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>(ui);

  ui->on_change_page([this](AppPage page) { this->change_page(page); });

  ui->global<AppContext>().set_version(
    std::string_view{STRINGIFY(PROJECT_VERSION)});
  ui->global<AppContext>().set_version_suffix("experimental");

  change_page(ui->get_active_page());

  /// \todo Add a timeout
  /// \todo Find a more elegant solution than just aborting the process
  ui->window().on_close_requested([this]() {
    this->_pManager->abort();
    while (_pManager->busy()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500uz));
    }
    return slint::CloseRequestResponse::HideWindow;
  });
}

/**
 * Change page callback (UI)
 *
 * \details
 * This will issue a 'connect' to the corresponding page backend
 *
 * \param page New page
 */
void AppBackend::change_page(AppPage page) {
  if (auto ui{_weakUi.lock()}) {
    _pManager->connect((*ui));
    switch (page) {
      case AppPage::Update: _updateBackend->connect(*ui); break;
      case AppPage::Soundload: _soundLoadBackend->connect(*ui); break;
      case AppPage::About: break;
      default: assert(false);
    }
  }
}

} // namespace ui
