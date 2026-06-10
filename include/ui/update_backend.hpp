#pragma once

#include <app-window.h>
#include <slint.h>
#include <chrono>
#include "i_backend.hpp"
#include "include/process/i_update_process.hpp"
#include "include/process/mdu_ein/update.hpp"
#include "include/type/step.hpp"
#include "include/ui/helper/progress_tracker.hpp"

namespace ui {

struct UpdateBackend : IBackend {
  UpdateBackend() = default;
  virtual ~UpdateBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow>);

private:
  void choose_file();
  void start_process();
  void abort_process();

  void done();

  void updateStep(type::UpdateStep const step);
  void updateProgress(double progress);

  void updateUi(std::function<void()>);

  slint::ComponentWeakHandle<AppWindow> _weakUi{};

  std::shared_ptr<process::IUpdateProcess> _process{};

  type::UpdateStep _step{type::UpdateStep::Done};

  std::filesystem::path _path{};

  helper::ProgressTracker _tracker{};
};

} // namespace ui
