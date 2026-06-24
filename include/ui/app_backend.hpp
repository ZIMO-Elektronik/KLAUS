/**
 * Main app backend
 *
 * \file    include/ui/app_backend.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

#include <slint.h>
#include "i_backend.hpp"
#include "process_manager.hpp"
#include "soundload_backend.hpp"
#include "update_backend.hpp"

namespace ui {

/**
 * Main app backend
 *
 * \details
 * Sortof works like a connection dispatcher and initial-state setup.a
 *
 * It's main responsibility is to re-connect the UI when the page changes to
 * avoid having to create each callback for every occurrence in a page
 *
 */
struct AppBackend : IBackend {
  AppBackend();
  virtual ~AppBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow> ui);

  void change_page(AppPage index);

private:
  slint::ComponentWeakHandle<AppWindow> _weakUi{}; ///< WeakHandle to UI

  std::shared_ptr<ProcessManager> _pManager{}; ///< Process Manager

  std::unique_ptr<UpdateBackend> _updateBackend{}; ///< Update page backend
  std::unique_ptr<SoundLoadBackend>
    _soundLoadBackend{}; ///< SoundLoad page backend
};

} // namespace ui
