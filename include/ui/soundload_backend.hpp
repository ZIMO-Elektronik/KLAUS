#pragma once

#include <app-window.h>
#include <slint.h>
#include "i_backend.hpp"
#include "include/process/i_update_process.hpp"
#include "include/process/susiv2/soundload.hpp"
#include "include/type/step.hpp"

namespace ui {

struct SoundLoadBackend : IBackend {
  SoundLoadBackend() = default;
  virtual ~SoundLoadBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow>);

private:
  void choose_file();
  void start_process();
  void abort_process();

  void done();

  void updateStep(type::SoundLoadStep const step);
  void updateProgress(double progress);

  void updateUi(std::function<void()>);

  SoundLoadMode _mode{SoundLoadMode::ZUSI};

  slint::ComponentWeakHandle<AppWindow> _weakUi{};

  std::shared_ptr<process::ISoundLoadProcess> _process{};

  type::SoundLoadStep _step{type::SoundLoadStep::Done};

  std::filesystem::path _path{};
};

} // namespace ui
