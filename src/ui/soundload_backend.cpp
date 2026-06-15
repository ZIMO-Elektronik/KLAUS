#include "include/ui/soundload_backend.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "include/process/mdu_ein/soundload.hpp"
#include "include/process/susiv2/soundload.hpp"

namespace ui {

SoundLoadBackend::SoundLoadBackend(std::shared_ptr<ProcessManager> pManager)
  : _pManager{pManager} {}

void SoundLoadBackend::connect(slint::ComponentHandle<AppWindow> window) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>{window};

  window->on_sound_mode_changed(
    [this](SoundLoadMode mode) { this->_mode = mode; });
  window->on_choose_file([this]() { this->choose_file(); });
  window->on_start_process([this]() { this->start_process(); });
  window->on_abort_process([this]() { this->abort_process(); });

  window->set_step_name({"Awaiting start"});
  window->set_progress_value(0.0);

  auto const has_file{this->_path.has_filename()};
  if (has_file) {
    window->set_has_file(true);
    window->set_file_path({_path.string().data()});
  } else {
    window->set_has_file(false);
  }
}

void SoundLoadBackend::choose_file() {
  // 1. Filter für den Dialog definieren
  // tinyfiledialogs erwartet ein Array aus Zeichenketten für die Endungen
  char const* filterPatterns[] = {"*.zpp"};

  // 2. Den Datei-Öffnen-Dialog aufrufen
  char const* selectedPath = tinyfd_openFileDialog(
    "ZPP-Datei auswählen", // Dialog-Titel
    "",                    // Standard-Pfad (leer = aktuelles Verzeichnis)
    1,                     // Anzahl der Filter-Muster im Array
    filterPatterns,        // Das Filter-Array
    "ZPP Dateien (*.zpp)", // Beschreibung des Filters für den Nutzer
    0                      // 0 = Nur eine Datei auswählbar, 1 = Mehrfachauswahl
  );

  // 3. Überprüfen, ob der Nutzer die Auswahl abgebrochen hat
  if (!selectedPath) {
    std::cout << "Auswahl wurde abgebrochen.\n";
    return;
  }

  // 4. In std::filesystem::path konvertieren
  std::filesystem::path zppPath(selectedPath);

  // 5. Nutzen des Pfads (Beispiel-Ausgabe)
  std::cout << "Erfolgreich ausgewählt!\n";
  std::cout << "Absoluter Pfad: " << zppPath << "\n";
  std::cout << "Dateiname:      " << zppPath.filename() << "\n";

  _path = zppPath;

  if (auto ui{_weakUi.lock()}) {
    (*ui)->set_has_file(true);
    (*ui)->set_file_path(_path.string().data());
  }

  return;
}

void SoundLoadBackend::start_process() {
  switch (_mode) {
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

void SoundLoadBackend::abort_process() { _pManager->abort(); }

} // namespace ui
