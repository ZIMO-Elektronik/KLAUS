#include <cstdlib>
#include <iostream>
#include "app-window.h"
#include "include/ui/cv/cv_programmer.hpp"

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

  auto ui = AppWindow::create();

  ui::cv::CvProgrammer programmer{};
  programmer.connect(ui);

  ui->run();

  programmer.check();

  return 0;
}
