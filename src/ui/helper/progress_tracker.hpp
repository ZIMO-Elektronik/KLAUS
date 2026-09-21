/**
 * Progress tracker
 *
 * \file    include/ui/helper/progress_tracker.hpp
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
