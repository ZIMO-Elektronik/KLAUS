#pragma once

#include <app-window.h>
#include <slint.h>
#include "i_backend.hpp"
#include "include/process/susiv2/soundload.hpp"
#include "include/type/step.hpp"
#include "include/ui/helper/progress_tracker.hpp"
#include "include/ui/process_manager.hpp"

namespace ui {

struct SoundLoadBackend : IBackend {
  SoundLoadBackend() = default;
  SoundLoadBackend(std::shared_ptr<ProcessManager> pManager);
  virtual ~SoundLoadBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow>);

private:
  void choose_file();
  void start_process();

  SoundLoadMode _mode{SoundLoadMode::ZUSI};

  slint::ComponentWeakHandle<AppWindow> _weakUi{};

  std::shared_ptr<ProcessManager> _pManager{std::make_shared<ProcessManager>()};

  type::SoundLoadStep _step{type::SoundLoadStep::Done};

  std::filesystem::path _path{};
  std::shared_ptr<libklug::ZPP> _zpp{};

  helper::ProgressTracker _tracker{};
};

} // namespace ui
