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
 * Main
 *
 * \file    main.cpp
 * \author  Jonas Gahlert
 * \date    21.09.2026
 */

#include <cstdlib>
#include <iostream>
#include "app-window.h"
// #include "ui/cv/cv_programmer.hpp"

#include "process/mdu_ein/update.hpp"
#include "ui/app_backend.hpp"

#ifdef _WIN32
#  include <windows.h>

/**
 * Check if the program is launched in a VM
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

  ui::AppBackend app{};
  app.connect(ui);

  ui->run();

  return 0;
}
