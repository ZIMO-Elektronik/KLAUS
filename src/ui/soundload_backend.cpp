#include "include/ui/soundload_backend.hpp"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "include/process/mdu_ein/soundload.hpp"
#include "include/process/susiv2/soundload.hpp"

namespace ui {

void SoundLoadBackend::connect(slint::ComponentHandle<AppWindow> window) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>{window};

  window->on_sound_mode_changed(
    [this](SoundLoadMode mode) { this->_mode = mode; });
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
  if (auto ui{_weakUi.lock()}) { (*ui)->set_show_progress(true); }

  switch (_mode) {
    case SoundLoadMode::ZUSI: {
      auto tmp_ = std::make_shared<process::susiv2::SoundLoad>(_path);
      tmp_->setup(tmp_);
      _process = tmp_;
      break;
    }
    case SoundLoadMode::MDU: {
      auto tmp_ = std::make_shared<process::mdu_ein::SoundLoad>(_path);
      tmp_->setup(tmp_);
      _process = tmp_;
      break;
    }
  }
  _process->onUpdateStep(
    [this](type::SoundLoadStep const step) { this->updateStep(step); });
  _process->onUpdateProgress(
    [this](double progress) { this->updateProgress(progress); });

  _process->execute();
  _tracker.reset();
}

void SoundLoadBackend::abort_process() { _process->abort(); }

void SoundLoadBackend::done() { _process.reset(); }

void SoundLoadBackend::updateProgress(double progress) {
  using std::operator""sv;
  updateUi([this, progress]() {
    std::string_view str{_tracker.update(progress) ? _tracker.estimate()
                                                   : ""sv};
    if (auto ui{this->_weakUi.lock()}) {
      (*ui)->set_progress_value(progress);
      if (!str.empty()) (*ui)->set_progress_estimate(str.data());
    }
  });
}

void SoundLoadBackend::updateStep(type::SoundLoadStep const step) {
  if (step == _step) return;

  switch (step) {
    case type::SoundLoadStep::Start:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Started"});
        }
      });
      break;
    case type::SoundLoadStep::Init:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Initializing"});
        }
      });
      break;
    case type::SoundLoadStep::Search:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Searching decoder"});
        }
      });
      break;
    case type::SoundLoadStep::Erase:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Erasing Flash"});
        }
      });
      break;
    case type::SoundLoadStep::Load:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Writing update"});
        }
      });
      break;
    case type::SoundLoadStep::Cleanup:
      updateUi([this]() {
        if (auto ui{this->_weakUi.lock()}) {
          (*ui)->set_step_name({"Cleanup"});
        }
      });
      break;
    case type::SoundLoadStep::Done:
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

void SoundLoadBackend::updateUi(std::function<void()> fn) {
  slint::invoke_from_event_loop(fn);
}

} // namespace ui
