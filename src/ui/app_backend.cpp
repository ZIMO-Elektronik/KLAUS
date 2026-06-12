#include "include/ui/app_backend.hpp"

namespace ui {

AppBackend::AppBackend()
  : _pManager{std::make_shared<ProcessManager>()},
    _updateBackend{std::make_unique<UpdateBackend>(_pManager)},
    _soundLoadBackend{std::make_unique<SoundLoadBackend>()} {}

void AppBackend::connect(slint::ComponentHandle<AppWindow> ui) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>(ui);

  ui->on_change_page([this](AppPage page) { this->change_page(page); });

  change_page(ui->get_active_page());
}

void AppBackend::change_page(AppPage page) {
  if (auto ui{_weakUi.lock()}) {
    _pManager->connect((*ui));
    switch (page) {
      case AppPage::Update: _updateBackend->connect(*ui); break;
      case AppPage::Soundload: _soundLoadBackend->connect(*ui); break;
      case AppPage::CvProgrammer: break;
      default: assert(false);
    }
  }
}

} // namespace ui
