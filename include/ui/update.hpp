#pragma once

#include <app-window.h>
#include <slint.h>
#include "i_backend.hpp"
#include "include/process/i_update_process.hpp"
#include "include/process/mdu_ein/update.hpp"
#include "include/type/step.hpp"

namespace ui {

struct Update : IBackend {
  Update() = default;
  virtual ~Update() = default;

  virtual void connect(slint::ComponentHandle<AppWindow>);

private:
  void choose();

  void start();
  void abort();

  void done();

  void updateStep(type::UpdateStep const step);
  void updateProgress(double progress);

  void updateUi(std::function<void()>);

  slint::ComponentWeakHandle<AppWindow> _weakUi{};

  std::shared_ptr<process::IUpdateProcess> _process{};

  type::UpdateStep _step{type::UpdateStep::Done};

  std::filesystem::path _path{};
};

} // namespace ui
