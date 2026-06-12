#pragma once

#include <slint.h>
#include "i_backend.hpp"
#include "process_manager.hpp"
#include "soundload_backend.hpp"
#include "update_backend.hpp"

namespace ui {

struct AppBackend : IBackend {
  AppBackend();
  virtual ~AppBackend() = default;

  virtual void connect(slint::ComponentHandle<AppWindow> ui);

  void change_page(AppPage index);

private:
  slint::ComponentWeakHandle<AppWindow> _weakUi{};

  std::shared_ptr<ProcessManager> _pManager{};

  std::unique_ptr<UpdateBackend> _updateBackend{};
  std::unique_ptr<SoundLoadBackend> _soundLoadBackend{};
};

} // namespace ui
