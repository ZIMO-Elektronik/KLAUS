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
 * \file    ui/helper/progress_tracker.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

namespace ui::helper {

/**
 * ProgressTracker
 *
 * \details
 * Tracks an estimated remaining time of a continuous process.
 *
 * Using \ref ProgressTracker::update, the latest progress mark (0.0 - 1.0) can
 * be updated. This will only have an effect once per second, unless `force` is
 * set to `true`.
 *
 * \ref ProgressTracker::estimate will return the last time estimate as a string
 * "hh:mm:ss"
 *
 * \todo
 * Insert an array of 5 or so points to be able to make a window estimate over a
 * vew seconds. May be more precise than what were doing right now.
 *
 */
struct ProgressTracker {
  void reset();

  bool update(double progress, bool force = false);
  std::string_view estimate();

private:
  std::string _str{}; ///< Created string

  double _lastProgress{0.0f}; ///< Last progress value
  std::chrono::steady_clock::time_point _lastUpdate{
    std::chrono::steady_clock::now()}; ///< Last update point
};

} // namespace ui::helper
