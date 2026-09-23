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
 * Update backend
 *
 * \file    ui/update_backend.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "ui/update_backend.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "ui/helper/firmware_fetcher.hpp"

namespace ui {

/**
 * CTor
 *
 * \param pManager ProcessManager pointer
 */
UpdateBackend::UpdateBackend(std::shared_ptr<ProcessManager> pManager)
  : _pManager{pManager} {}

/**
 * Connect (UI)
 *
 * \param window UI handle
 */
void UpdateBackend::connect(slint::ComponentHandle<AppWindow> window) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>{window};

  window->on_choose_file([this]() { this->choose_file(); });
  window->on_fetch_file([this]() { this->fetch_file(); });
  window->on_start_process([this]() { this->start_process(); });

  auto model{window->get_firmwares()};
  if (model == nullptr) { // Create model if it does not exist
    model = std::make_shared<slint::VectorModel<FirmwareAdapter>>();
    window->set_firmwares(model);
  }

  auto const has_file{this->_path.has_filename()};
  if (has_file) {
    window->set_has_file(true);
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
void UpdateBackend::choose_file() {
  char const* filterPatterns[] = {"*.zsu"};

  char const* selectedPath =
    tinyfd_openFileDialog("Select a ZSU file", // Dialog-Title
                          "",                  // Standard-path (empty == cwd)
                          1,                   // Filter count
                          filterPatterns,      // Filter array
                          "ZSU files (*.zsu)", // Filter description
                          0                    // 0 = Only one file selectable
    );

  // Check if the user has aborted the selection
  if (!selectedPath) {
    std::cout << "Selection aborted." << std::endl;
    return;
  }

  std::filesystem::path zsuPath(selectedPath);

  // Print path
  std::cout << "Selected\n";
  std::cout << "Absolute path: " << zsuPath << "\n";
  std::cout << "File name:     " << zsuPath.filename() << "\n";

  _path = zsuPath;

  open_file();
}

void UpdateBackend::fetch_file() {
  _path = helper::FirmwareFetcher::fetchLatest();
  open_file();
}

/**
 * Start process (UI)
 *
 * \details
 * Creates and starts a the selected process
 *
 */
void UpdateBackend::start_process() {
  /// Careful, we can only pass an empty list while the underlying process sends
  /// an entry on ID 0 in that case.
  auto decoder_ids{prepare_selected_firmware_list<true>()};

  EntryType entry_type{EntryType::MDU};

  if (auto ui{_weakUi.lock()}) { entry_type = (*ui)->get_entry_type(); }

  if (_pManager->emplace<process::mdu_ein::Update>(
        _zsu,
        entry_type == EntryType::MDU ? type::MDUEntryType::MDU
                                     : type::MDUEntryType::DCC_ZSU,
        std::move(decoder_ids)))
    _pManager->execute();
  else std::cerr << "Manager is busy" << std::endl;
}

void UpdateBackend::open_file() {
  _zsu = std::make_shared<libulf::ZSU>(_path);
  if (!_zsu->valid()) {
    // Cant read file
    std::cerr << "Unable to read file";
    _zsu.reset();
    return;
  }

  create_firmware_list();

  if (auto ui{_weakUi.lock()}) { (*ui)->set_has_file(true); }

  return;
}

/**
 * Create firmware list
 *
 * \details
 * This iterates through all available firmwares in the ZSU file and creates a
 * lexicographically sorted list.
 *
 */
void UpdateBackend::create_firmware_list() {
  auto model{std::make_shared<slint::VectorModel<FirmwareAdapter>>()};

  auto iter{_zsu->begin()};
  auto const end{_zsu->end()};

  do {
    auto id_model{std::make_shared<slint::VectorModel<int>>()};
    auto const id{iter.id()};

    for (uint8_t i{0u}; i < sizeof(uint32_t); i++) {
      int byte_val = static_cast<int>((id >> (i * 8)) & 0xFF);
      id_model->push_back(byte_val);
    }

    model->push_back(FirmwareAdapter{.decoder_name = iter.name().substr(
                                       0, iter.name().find_first_of("-") + 2uz),
                                     .decoder_id = id_model,
                                     .major_version = iter.versionMajor(),
                                     .minor_version = iter.versionMinor()});
  } while (++iter != end);

  auto filter_model{std::make_shared<slint::SortModel<FirmwareAdapter>>(
    model, [](FirmwareAdapter const& lhs, FirmwareAdapter const& rhs) {
      return std::lexicographical_compare(
        std::string_view(lhs.decoder_name).begin(),
        std::string_view(lhs.decoder_name).end(),
        std::string_view(rhs.decoder_name).begin(),
        std::string_view(rhs.decoder_name).end(),
        [](unsigned char c1, unsigned char c2) {
          return std::tolower(c1) < std::tolower(c2);
        });
    })};

  if (auto ui{_weakUi.lock()}) {
    (*ui)->set_firmwares(model);
    (*ui)->set_filtered_firmwares(filter_model);
  }
}

} // namespace ui
