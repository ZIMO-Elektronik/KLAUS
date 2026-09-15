#pragma once

#include <app-window.h>
#include <slint.h>
#include <chrono>
#include <ulf/cpp/libulf.hpp>
#include "i_backend.hpp"
#include "include/process/mdu_ein/update.hpp"
#include "include/type/step.hpp"
#include "include/ui/helper/progress_tracker.hpp"
#include "include/ui/process_manager.hpp"

namespace ui {

/**
 * Update page backend
 *
 * \details
 * Handles the events of the Update page.
 *
 * To connect to the UI, use the \ref UpdateBackend::connect method.
 *
 * This class will register the following callbacks on the UI:
 * | Method | Callback |
 * | -------------- | - |
 * | choose_file    |   |
 * | start_process  |   |
 *
 */
struct UpdateBackend : IBackend {
  UpdateBackend() = default;
  UpdateBackend(std::shared_ptr<ProcessManager> pManager);
  virtual ~UpdateBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow>);

private:
  void choose_file();
  void start_process();

  void create_firmware_list();

  template<bool check_selected>
  std::vector<uint32_t> prepare_selected_firmware_list() {
    std::vector<uint32_t> decoder_ids;

    if (auto ui{_weakUi.lock()}) {
      auto model{(*ui)->get_firmwares()};

      for (size_t i{0}; i < model->row_count(); i++) {
        if (auto const row{model->row_data(i)}) {
          if constexpr (check_selected) {
            if (!row->selected) continue;
          }

          assert(row->decoder_id->row_count() == 4);
          uint32_t id{0};

          for (uint8_t i{0u}; i < sizeof(uint32_t); i++) {
            if (auto const id_row{row->decoder_id->row_data(i)}) {
              id |= (static_cast<uint32_t>(*id_row) & 0xFF) << (i * 8);
            } else {
              assert(false);
            }
          }
          decoder_ids.push_back(id);
        }
      }
    }
    return decoder_ids; // std::move ist bei lokalen Variablen (NRVO) nicht
                        // nötig
  }

  slint::ComponentWeakHandle<AppWindow> _weakUi{}; ///< WeakHandle to the UI

  std::shared_ptr<ProcessManager> _pManager{
    std::make_shared<ProcessManager>()}; ///< ProcessManager

  std::filesystem::path _path{};       ///< Path to ZSU
  std::shared_ptr<libulf::ZSU> _zsu{}; ///< ZSU (from LibULF)

  helper::ProgressTracker _tracker{}; ///< ProgressTracker (for estimate)
};

} // namespace ui
