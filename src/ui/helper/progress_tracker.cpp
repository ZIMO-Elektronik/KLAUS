/**
 * Copyright (C) 2026 ZIMO Elektronik
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
 * Progress tracker
 *
 * \file    ui/helper/progress_tracker.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "ui/helper/progress_tracker.hpp"
#include <iomanip>

namespace ui::helper {

/**
 * Reset
 *
 */
void ProgressTracker::reset() {
  _lastProgress = 0.0f;
  _str = "--:--:--";
  _lastUpdate = std::chrono::steady_clock::now();
}

/**
 * Update estimate
 *
 * \details
 * Will create an estimate based on the progress since the last update.
 *
 * For now, this has a fixed 1 second interval, unless the 'force' option is
 * set, in which case an update will be made
 *
 * \param progress  New progress
 * \param force     Force update
 *
 * \return true   Estimate was updated
 * \return false  Estimate not updated
 */
bool ProgressTracker::update(double progress, bool force) {
  auto const now{std::chrono::steady_clock::now()};
  if (now < (_lastUpdate + std::chrono::milliseconds(1000))) return false;

  // Update estimate
  if (progress <= 0.001f) _str = "--:--:--";
  else if (progress >= 1.0f) _str = "00:00:00";
  else {
    float const delta = progress - _lastProgress;
    if (delta < 0.0f) {
      _str = "--:--:--";
      _lastProgress = progress;

    } else {
      auto const remaining_progress = 1.0f - progress;

      auto remaining_duration{
        std::chrono::seconds(static_cast<int>(remaining_progress / delta))};

      auto const hours{
        std::chrono::duration_cast<std::chrono::hours>(remaining_duration)};
      remaining_duration -= hours;

      auto const minutes{
        std::chrono::duration_cast<std::chrono::minutes>(remaining_duration)};
      remaining_duration -= minutes;

      auto const seconds{
        std::chrono::duration_cast<std::chrono::seconds>(remaining_duration)};

      std::ostringstream oss;
      oss << std::setfill('0') << std::setw(2) << hours.count() << ":"
          << std::setw(2) << minutes.count() << ":" << std::setw(2)
          << seconds.count();

      _str = oss.str();
    }
  }

  _lastProgress = progress;
  _lastUpdate = std::chrono::steady_clock::now();

  return true;
}

/// Returns the current estimate
std::string_view ProgressTracker::estimate() { return {_str}; }

} // namespace ui::helper
