#include "include/ui/app_backend.hpp"

#define STRINGIFY(x) TO_STRING(x)
#define TO_STRING(x) #x

namespace ui {

AppBackend::AppBackend()
  : _pManager{std::make_shared<ProcessManager>()},
    _updateBackend{std::make_unique<UpdateBackend>(_pManager)},
    _soundLoadBackend{std::make_unique<SoundLoadBackend>(_pManager)} {}

void AppBackend::connect(slint::ComponentHandle<AppWindow> ui) {
  _weakUi = slint::ComponentWeakHandle<AppWindow>(ui);

  ui->on_change_page([this](AppPage page) { this->change_page(page); });

  ui->global<AppContext>().set_version(
    std::string_view{STRINGIFY(PROJECT_VERSION)});
  ui->global<AppContext>().set_version_suffix("experimental");

  change_page(ui->get_active_page());
}

void AppBackend::change_page(AppPage page) {
  if (auto ui{_weakUi.lock()}) {
    _pManager->connect((*ui));
    switch (page) {
      case AppPage::Update: _updateBackend->connect(*ui); break;
      case AppPage::Soundload: _soundLoadBackend->connect(*ui); break;
      case AppPage::About: break;
      default: assert(false);
    }
  }
}

} // namespace ui
