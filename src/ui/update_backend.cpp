#include "include/ui/update_backend.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>

namespace ui {

UpdateBackend::UpdateBackend(std::shared_ptr<ProcessManager> pManager)
  : _pManager{pManager} {}

void UpdateBackend::connect(slint::ComponentHandle<AppWindow> window) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>{window};

  window->on_choose_file([this]() { this->choose_file(); });
  window->on_start_process([this]() { this->start_process(); });

  auto model{window->get_firmwares()};
  if (model == nullptr) { // Create model if it does not exist
    model = std::make_shared<slint::VectorModel<FirmwareAdapter>>();
    window->set_firmwares(model);
  }

  auto const has_file{this->_path.has_filename()};
  if (has_file) {
    window->set_has_file(true);
    window->set_file_path({_path.string().data()});
  } else {
    window->set_has_file(false);
  }
}

void UpdateBackend::choose_file() {
  // 1. Filter für den Dialog definieren
  // tinyfiledialogs erwartet ein Array aus Zeichenketten für die Endungen
  char const* filterPatterns[] = {"*.zsu"};

  // 2. Den Datei-Öffnen-Dialog aufrufen
  char const* selectedPath = tinyfd_openFileDialog(
    "ZSU-Datei auswählen", // Dialog-Titel
    "",                    // Standard-Pfad (leer = aktuelles Verzeichnis)
    1,                     // Anzahl der Filter-Muster im Array
    filterPatterns,        // Das Filter-Array
    "ZSU Dateien (*.zsu)", // Beschreibung des Filters für den Nutzer
    0                      // 0 = Nur eine Datei auswählbar, 1 = Mehrfachauswahl
  );

  // 3. Überprüfen, ob der Nutzer die Auswahl abgebrochen hat
  if (!selectedPath) {
    std::cout << "Auswahl wurde abgebrochen.\n";
    return;
  }

  // 4. In std::filesystem::path konvertieren
  std::filesystem::path zsuPath(selectedPath);

  // 5. Nutzen des Pfads (Beispiel-Ausgabe)
  std::cout << "Erfolgreich ausgewählt!\n";
  std::cout << "Absoluter Pfad: " << zsuPath << "\n";
  std::cout << "Dateiname:      " << zsuPath.filename() << "\n";

  _path = zsuPath;

  _zsu = std::make_shared<libklug::ZSU>(_path);
  if (!_zsu->valid()) {
    // Cant read file
    std::cerr << "Unable to read file";
    _zsu.reset();
    return;
  }

  create_firmware_list();

  if (auto ui{_weakUi.lock()}) {
    (*ui)->set_has_file(true);
    (*ui)->set_file_path(_path.string().data());
  }

  return;
}

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

    model->push_back(FirmwareAdapter{.decoder_name = iter.name(),
                                     .decoder_id = id_model,
                                     .major_version = iter.versionMajor(),
                                     .minor_version = iter.versionMinor()});
  } while (++iter != end);

  if (auto ui{_weakUi.lock()}) { (*ui)->set_firmwares(model); }
}

} // namespace ui
