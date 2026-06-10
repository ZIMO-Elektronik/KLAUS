#include "include/ui/update_backend.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>

namespace ui {

void UpdateBackend::connect(slint::ComponentHandle<AppWindow> window) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>{window};

  window->on_choose_file([this]() { this->choose_file(); });
  window->on_start_process([this]() { this->start_process(); });
  window->on_abort_process([this]() { this->abort_process(); });

  window->set_show_progress(static_cast<bool>(_process));

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

  if (auto ui{_weakUi.lock()}) {
    (*ui)->set_has_file(true);
    (*ui)->set_file_path(_path.string().data());
  }

  return;
}

void UpdateBackend::start_process() {
  if (auto ui{_weakUi.lock()}) { (*ui)->set_show_progress(true); }

  _process = std::make_shared<process::mdu_ein::Update>(_path);
  _process->onUpdateStep(
    [this](type::UpdateStep const step) { this->updateStep(step); });
  _process->onUpdateProgress(
    [this](double progress) { this->updateProgress(progress); });

  _process->execute();

  _tracker.reset();
}

void UpdateBackend::abort_process() { _process->abort(); }

void UpdateBackend::done() { _process.reset(); }

void UpdateBackend::updateProgress(double progress) {
  using std::operator""sv;
  updateUi([this, progress]() {
    std::string_view str{_tracker.update(progress) ? _tracker.estimate()
                                                   : ""sv};
    if (auto ui{this->_weakUi.lock()}) {
      (*ui)->set_progress_value(progress);
      if (!str.empty()) (*ui)->set_progress_estimate({str.data()});
    }
  });
}

void UpdateBackend::updateStep(type::UpdateStep const step) {
  if (step == _step) return;

  switch (step) {
    case type::UpdateStep::Start:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Started"});
        }
      });
      break;
    case type::UpdateStep::Init:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Initializing"});
        }
      });
      break;
    case type::UpdateStep::Search:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Searching decoder"});
        }
      });
      break;
    case type::UpdateStep::Erase:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Erasing Flash"});
        }
      });
      break;
    case type::UpdateStep::Update:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Writing update"});
        }
      });
      break;
    case type::UpdateStep::Verify:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Verifying"});
        }
      });
      break;
    case type::UpdateStep::Cleanup:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Cleanup"});
        }
      });
      break;
    case type::UpdateStep::Done:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Done"});
          (*ui)->set_show_progress(false);
        }
        this->done();
      });
      break;
  }
  _step = step;
}

void UpdateBackend::updateUi(std::function<void()> fn) {
  slint::invoke_from_event_loop(fn);
}

} // namespace ui
