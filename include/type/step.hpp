/**
 * Process Step enums
 *
 * \file    include/type/step.hpp
 * \author  Jonas Gahlert
 * \date    22.05.2026
 */

#pragma once

namespace type {

/**
 * Update (ZSU) steps
 *
 */
enum class UpdateStep {
  Start,
  Search,
  Init,
  Erase,
  Update,
  Verify,
  Cleanup,
  Done,
};

/**
 * Soundload (ZPP) steps
 *
 */
enum class SoundLoadStep {
  Start,
  Search,
  Init,
  Erase,
  Load,
  Cleanup,
  Done,
};

/**
 * Cv Read / Write Steps
 *
 */
enum class CvStep {
  Start,
  CvRead,
  CvWrite,
  Cleanup,
  Done,
};

} // namespace type
