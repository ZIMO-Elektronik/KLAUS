/**
 * Process Step enums
 *
 * \file    include/type/step.hpp
 * \author  Jonas Gahlert
 * \date    22.05.2026
 */

#pragma once

#include <optional>
#include <variant>

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

enum class MessageID {
  Start,
  StartComplete,
  StartError,
  Init,
  InitComplete,
  SearchDecoder,
  FoundDecoder,
  EraseFlash,
  EraseFlashComplete,
  WriteFlash,
  WriteFlashComplete,
  ReadCv,
  ReadCvComplete,
  WriteCv,
  WriteCvComplete,
  Verify,
  VerifyComplete,
  VerifyError,
  Cleanup,
  CleanupComplete,
  Done,

  None,
};

using Payload = std::variant<std::monostate>;

struct ProcessUpdate {
  MessageID id{MessageID::None};
  std::optional<float> progress{};
  Payload payload{};
};

} // namespace type
