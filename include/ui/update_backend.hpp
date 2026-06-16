#pragma once

#include <app-window.h>
#include <slint.h>
#include <chrono>
#include <libklug/libklug.hpp>
#include "i_backend.hpp"
#include "include/process/mdu_ein/update.hpp"
#include "include/type/step.hpp"
#include "include/ui/helper/progress_tracker.hpp"
#include "include/ui/process_manager.hpp"

namespace ui {

struct UpdateBackend : IBackend {
  UpdateBackend() = default;
  UpdateBackend(std::shared_ptr<ProcessManager> pManager);
  virtual ~UpdateBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow>);

private:
  void choose_file();
  void start_process();

  void create_firmware_list();

  slint::ComponentWeakHandle<AppWindow> _weakUi{};

  std::shared_ptr<ProcessManager> _pManager{std::make_shared<ProcessManager>()};

  type::UpdateStep _step{type::UpdateStep::Done};

  std::filesystem::path _path{};
  std::shared_ptr<libklug::ZSU> _zsu{};

  helper::ProgressTracker _tracker{};
};

} // namespace ui
