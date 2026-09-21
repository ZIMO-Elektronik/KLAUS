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
 * MDU entry type
 *
 * \details
 * What does the doc say? Enum matches its MDU entry counterpart
 *
 */
enum class MDUEntryType {
  MDU,     ///< MDU (PowerCycle) entry
  DCC_ZSU, ///< DCC ZSU (OpsMode) entry
  DCC_ZPP, ///< DCC ZPP (OpsMode) entry
};

/**
 * UI Update message ID
 *
 * \details
 * This is a way to push UI updates without having to place strings in the
 * backend.. Not elegant but it works.
 *
 * \note
 * Since the problems with gettext are seemingly unsolvable, this may become
 * obsolete in favor of a project fluent based translation system.
 *
 */
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

  Abort,
  AbortInit,
  AbortDecoderSearch,
  AbortFlashErase,
  AbortFlashWrite,
  AbortVerify,

  AbortUnresponsive,

  None,
};

using DeviceString = std::string;

/**
 * A payload
 *
 * \details
 * As of now, this is only used to signal the end of the process.
 *
 */
using Payload = std::variant<std::monostate, bool, DeviceString>;

/**
 * Process update struct
 *
 * \details
 * This is used to send state updates from a background process to the gui
 * without having to worry about strings.
 *
 */
struct ProcessUpdate {
  MessageID id{MessageID::None};   ///< ID of message
  std::optional<float> progress{}; ///< Progress (if any)
  Payload payload{};               ///< Payload (if any)
};

} // namespace type
