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
 * SoundLoad backend
 *
 * \file    ui/soundload_backend.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "ui/soundload_backend.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "process/mdu_ein/soundload.hpp"
#include "process/susiv2/soundload.hpp"

namespace ui {

/**
 * CTor
 *
 * \param pManager ProcessManager pointer
 */
SoundLoadBackend::SoundLoadBackend(std::shared_ptr<ProcessManager> pManager)
  : _pManager{pManager} {}

/**
 * Connect (to UI)
 *
 * \param window UI handle
 */
void SoundLoadBackend::connect(slint::ComponentHandle<AppWindow> window) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>{window};

  window->on_choose_file([this]() { this->choose_file(); });
  window->on_start_process([this]() { this->start_process(); });

  auto const has_file{this->_zpp != nullptr};
  if (has_file) {
    window->set_has_file(true);
    window->set_zpp_name(_path.filename().generic_string().data());
    window->set_zpp_author(_zpp->author());
    window->set_zpp_email(_zpp->email());
  } else {
    window->set_has_file(false);
  }
}

/**
 * Choose file (UI)
 *
 * \details
 * Starts a blocking native file dialog and handles the result
 *
 */
void SoundLoadBackend::choose_file() {
  char const* filterPatterns[] = {"*.zpp"};

  char const* selectedPath =
    tinyfd_openFileDialog("Select a ZPP file", // Dialog-Title
                          "",                  // Standard-path (empty == cwd)
                          1,                   // Filter count
                          filterPatterns,      // Filter array
                          "ZPP files (*.zpp)", // Filter description
                          0                    // 0 = Only one file selectable
    );

  // Check if the user has aborted the selection
  if (!selectedPath) {
    std::cout << "Selection aborted" << std::endl;
    return;
  }

  std::filesystem::path zppPath(selectedPath);

  // Print path
  std::cout << "Selected\n";
  std::cout << "Absolute path: " << zppPath << "\n";
  std::cout << "File name:     " << zppPath.filename() << "\n";

  _path = zppPath;

  _zpp = std::make_shared<libulf::ZPP>(_path);
  if (!_zpp->valid()) {
    // Cant read file
    std::cerr << "Unable to read file";
    _zpp.reset();
    return;
  }

  if (auto ui{_weakUi.lock()}) {
    (*ui)->set_has_file(true);
    (*ui)->set_zpp_name(_path.filename().generic_string().data());
    (*ui)->set_zpp_author(_zpp->author());
    (*ui)->set_zpp_email(_zpp->email());
  }

  return;
}

/**
 * Start process (UI)
 *
 * \details
 * Creates and starts a the selected process
 *
 */
void SoundLoadBackend::start_process() {
  SoundLoadMode mode{};
  if (auto ui{_weakUi.lock()}) { mode = (*ui)->get_sound_load_mode(); }

  switch (mode) {
    case SoundLoadMode::ZUSI:
      if (_pManager->emplace<process::susiv2::SoundLoad>(_path))
        _pManager->execute();
      else std::cerr << "Manager is busy" << std::endl;
      break;
    case SoundLoadMode::MDU:
      if (_pManager->emplace<process::mdu_ein::SoundLoad>(_path))
        _pManager->execute();
      else std::cerr << "Manager is busy" << std::endl;
      break;
  }
}

} // namespace ui
