#include <cstdlib>
#include <iostream>
#include "app-window.h"
// #include "include/ui/cv/cv_programmer.hpp"

#include "include/process/mdu_ein/update.hpp"
#include "include/ui/app_backend.hpp"

#ifdef _WIN32
#  include <windows.h>

/**
 * Check if the programm is launched in a VM
 *
 * \return true   Supported
 * \return false  Not supported
 */
bool check_for_vm() {
  // Check for Virtualbox
  DISPLAY_DEVICEA dd;
  dd.cb = sizeof(dd);
  DWORD deviceNum = 0;
  while (EnumDisplayDevicesA(NULL, deviceNum, &dd, 0)) {
    std::string deviceString(dd.DeviceString);
    if (deviceString.find("VirtualBox") != std::string::npos ||
        deviceString.find("VBox") != std::string::npos) {
      return false; // Most likely a VM
    }
    deviceNum++;
  }
  return true; // Probably not in a VM
}
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
  if (!check_for_vm()) {
    std::cout << "[Slint] VirtualBox/VM detected. Use Software Rendering. "
              << std::endl;
    _putenv_s("SLINT_BACKEND", "winit-software");
  } else {
    std::cout << "[Slint] Most likely not a VM. Try Skia backend (DirectX)."
              << std::endl;
    _putenv_s("SLINT_BACKEND", "winit-skia");
  }
#endif

  /*
  auto process{std::make_shared<process::mdu_ein::Update>()};
  process->setup(process);
   process->onUpdateProgress([](double progress) {
    int barWidth = 70;
    std::cout << "[";
    int pos = static_cast<int>(barWidth * progress);
    for (int j{0}; j < barWidth; j++) {
      if (j < pos) std::cout << "=";
      else if (j == pos) std::cout << ">";
      else std::cout << " ";
    }
    std::cout << "] Progress " << static_cast<int>(progress * 100) << "%";
    std::cout << "\r";
    std::cout.flush();

    if (progress >= 1) std::cout << std::endl;
  });

  bool done{false};
  type::UpdateStep last{type::UpdateStep::Done};

  process->onUpdateStep([&done, &last](type::UpdateStep step) {
    if (step == last) return;
    switch (step) {
      case type::UpdateStep::Start: std::cout << "Starting" << std::endl; break;
      case type::UpdateStep::Init:
        std::cout << "Initializing" << std::endl;
        break;
      case type::UpdateStep::Search:
        std::cout << "Searching Decoder" << std::endl;
        break;
      case type::UpdateStep::Erase:
        std::cout << "Erase Flash" << std::endl;
        break;
      case type::UpdateStep::Update:
        std::cout << "Updating" << std::endl;
        break;
      case type::UpdateStep::Verify:
        std::cout << "Verify Update" << std::endl;
        break;
      case type::UpdateStep::Cleanup:
        std::cout << "Cleanup" << std::endl;
        break;
      case type::UpdateStep::Done:
        std::cout << "Done" << std::endl;
        done = true;
        break;
    }
    last = step;
  });

  process->execute();

  while (!done) std::this_thread::sleep_for(std::chrono::seconds{2});

  process.reset();

  return 0;
  */

  auto ui = AppWindow::create();

  ui::AppBackend app{};
  app.connect(ui);

  ui->run();

  return 0;
}
