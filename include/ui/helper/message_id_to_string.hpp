/**
 * MessageID to string converter
 *
 * \file    include/ui/helper/message_id_to_string.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

// #include <libintl.h>
#include <string_view>
#include "include/type/step.hpp"

#define gettext(x) x

namespace ui::helper {

/**
 * Converts the given MessageID to its corresponding Message
 *
 * \note
 * Because of the problems with gettext, this may become obsolete in favor of a
 * fluent-based translation system
 *
 * \note
 * The `gettext` macros in here are currently only for show. They are resolved
 * by the macro above since we cant have libintl while cross-compiling
 *
 * \param id MessageID
 *
 * \return Message
 */
static std::string_view message_id_to_string(type::MessageID id) {
  using type::MessageID;

  switch (id) {
    case MessageID::Start: return gettext("Start");
    case MessageID::StartComplete: return gettext("Started");
    case MessageID::StartError: return gettext("Unable to start");
    case MessageID::Init: return gettext("Initializing");
    case MessageID::InitComplete: return gettext("Finished Initializing");
    case MessageID::SearchDecoder: return gettext("Search for decoder");
    case MessageID::FoundDecoder: return gettext("Found decoder");
    case MessageID::EraseFlash: return gettext("Erase Flash");
    case MessageID::EraseFlashComplete: return gettext("Erased Flash");
    case MessageID::WriteFlash: return gettext("Writing Flash");
    case MessageID::WriteFlashComplete: return gettext("Written Flash");
    case MessageID::ReadCv: return gettext("Reading CVs");
    case MessageID::ReadCvComplete: return gettext("Finished reading CVs");
    case MessageID::WriteCv: return gettext("Writing CVs");
    case MessageID::WriteCvComplete: return gettext("Finished writing CVs");
    case MessageID::Verify: return gettext("Verifying");
    case MessageID::VerifyComplete: return gettext("Successfully verified");
    case MessageID::VerifyError: return gettext("Error during verification");
    case MessageID::Cleanup: return gettext("Perform cleanup");
    case MessageID::CleanupComplete: return gettext("Cleanup complete");
    case MessageID::Done: return gettext("Done");
    case MessageID::Abort: return gettext("Aborted");
    case MessageID::AbortInit: return gettext("Unable to setup process");
    case MessageID::AbortDecoderSearch: return gettext("No decoder found");
    case MessageID::AbortFlashErase: return gettext("Unable to erase flash");
    case MessageID::AbortFlashWrite: return gettext("Unable to write flash");
    case MessageID::AbortVerify: return gettext("Error during verification");
    default: return gettext("Default");
  }
}

} // namespace ui::helper
