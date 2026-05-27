/**
 * Progress bar interface
 *
 * \file    include/ui/progress/i_progress_bar.hpp
 * \author  Jonas Gahlert
 * \date    26.05.2026
 */

#pragma once

namespace ui::progress {

/**
 * Progress bar interface
 *
 */
struct IProgressBar {
  virtual void attach() = 0;
  virtual void detach() = 0;
};

} // namespace ui::progress
