#pragma once

#include <string_view>
#include "include/type/step.hpp"

namespace ui::helper {

/**
 * Converts the given MessageID to its corresponding Message
 *
 * \param id MessageID
 *
 * \return Message
 */
static std::string_view message_id_to_string(type::MessageID id) {
  using type::MessageID;
  using std::operator""sv;

  switch (id) {
    case MessageID::Start: return "Start"sv;
    case MessageID::StartComplete: return "Started"sv;
    case MessageID::StartError: return "Unable to start"sv;
    case MessageID::Init: return "Initializing"sv;
    case MessageID::InitComplete: return "Finished Initializing"sv;
    case MessageID::SearchDecoder: return "Search for decoder"sv;
    case MessageID::FoundDecoder: return "Found decoder"sv;
    case MessageID::EraseFlash: return "Erase Flash"sv;
    case MessageID::EraseFlashComplete: return "Erased Flash"sv;
    case MessageID::WriteFlash: return "Writing Flash"sv;
    case MessageID::WriteFlashComplete: return "Written Flash"sv;
    case MessageID::ReadCv: return "Reading CVs"sv;
    case MessageID::ReadCvComplete: return "Finished reading CVs"sv;
    case MessageID::WriteCv: return "Writing CVs"sv;
    case MessageID::WriteCvComplete: return "Finished writing CVs"sv;
    case MessageID::Verify: return "Verifying"sv;
    case MessageID::VerifyComplete: return "Successfully verified"sv;
    case MessageID::VerifyError: return "Error during verification"sv;
    case MessageID::Cleanup: return "Perform cleanup"sv;
    case MessageID::CleanupComplete: return "Cleanup complete"sv;
    case MessageID::Done: return "Done"sv;
    default: return "Default"sv;
  }
}

} // namespace ui::helper
