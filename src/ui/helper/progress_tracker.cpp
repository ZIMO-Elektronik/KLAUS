/**
 * Progress tracker
 *
 * \file    src/ui/helper/progress_tracker.cpp
 * \author  Jonas Gahlert
 * \date    25.06.2026
 */

#include "include/ui/helper/progress_tracker.hpp"

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
