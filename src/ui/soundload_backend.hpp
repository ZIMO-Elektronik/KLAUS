/**
 * Copyright (C) 2026 [ZIMO Elektronik]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://gnu.org>.
 *
 *
 *
 *
 *
 * SoundLoad Backend
 *
 * \file    ui/soundload_backend.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

#include <app-window.h>
#include <slint.h>
#include "i_backend.hpp"
#include "process/susiv2/soundload.hpp"
#include "type/step.hpp"
#include "ui/helper/progress_tracker.hpp"
#include "ui/process_manager.hpp"

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

  std::filesystem::path _path{};       ///< ZPP path
  std::shared_ptr<libulf::ZPP> _zpp{}; ///< ZPP (from LibULF)

  helper::ProgressTracker _tracker{}; ///< Progress tracker (for estimate)
};

} // namespace ui
