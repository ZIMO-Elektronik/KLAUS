#include "include/ui/helper/progress_tracker.hpp"

namespace ui::helper {

void ProgressTracker::reset() {
  _lastProgress = 0.0f;
  _lastUpdate = std::chrono::steady_clock::now();
}

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

std::string_view ProgressTracker::estimate() { return {_str}; }

} // namespace ui::helper
