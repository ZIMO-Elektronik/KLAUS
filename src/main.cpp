#include "app-window.h"

// Dummy include
#include "include/ui/cv/cv_programmer.hpp"

int main(int argc, char** argv) {
  auto ui = AppWindow::create();

  ui::cv::CvProgrammer programmer{};
  programmer.connect(ui);

  ui->run();

  programmer.check();
  return 0;
}
