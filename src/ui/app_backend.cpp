#include "include/ui/app_backend.hpp"

namespace ui {

AppBackend::AppBackend() : _updateBackend{std::make_unique<UpdateBackend>()} {}

void AppBackend::connect(slint::ComponentHandle<AppWindow> ui) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>(ui);

  ui->on_change_page([this](AppPage page) { this->change_page(page); });

  change_page(ui->get_active_page());
}

void AppBackend::change_page(AppPage page) {
  if (auto ui{_weakUi.lock()}) {
    switch (page) {
      case AppPage::Update: _updateBackend->connect(*ui); break;
      case AppPage::Soundload: break;
      case AppPage::CvProgrammer: break;
      default: assert(false);
    }
  }
}

} // namespace ui
