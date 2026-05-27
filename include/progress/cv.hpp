#pragma once

#include "include/process/i_cv_process.hpp"

namespace ui::progress {

struct Cv {
  Cv(process::ICvProcess const& process);
  ~Cv() = default;

  virtual void attach();
  virtual void detach();
};

} // namespace ui::progress
