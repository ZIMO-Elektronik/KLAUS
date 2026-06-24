/**
 * SoundLoad Backend
 *
 * \file    include/ui/soundload_backend.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

#include <app-window.h>
#include <slint.h>
#include "i_backend.hpp"
#include "include/process/susiv2/soundload.hpp"
#include "include/type/step.hpp"
#include "include/ui/helper/progress_tracker.hpp"
#include "include/ui/process_manager.hpp"

namespace ui {

/**
 * SoundLoad page backend
 *
 * \details
 * Handles the events of the SoundLoad page.
 *
 * To connect to the UI, use the \ref SoundLoadBackend::connect method.
 *
 * This class will register the following callbacks on the UI:
 * | Method | Callback |
 * | -------------- | - |
 * | choose_file    |   |
 * | start_process  |   |
 *
 */
struct SoundLoadBackend : IBackend {
  SoundLoadBackend() = default;
  SoundLoadBackend(std::shared_ptr<ProcessManager> pManager);
  virtual ~SoundLoadBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow>);

private:
  void choose_file();
  void start_process();

  SoundLoadMode _mode{SoundLoadMode::ZUSI}; ///< SoundLoad mode (obsolete)

  slint::ComponentWeakHandle<AppWindow> _weakUi{}; ///< WeakHandle of the UI

  std::shared_ptr<ProcessManager> _pManager{
    std::make_shared<ProcessManager>()}; ///< ProcessManager

  std::filesystem::path _path{};        ///< ZPP path
  std::shared_ptr<libklug::ZPP> _zpp{}; ///< ZPP (from LibKLUG)

  helper::ProgressTracker _tracker{}; ///< Progress tracker (for estimate)
};

} // namespace ui
