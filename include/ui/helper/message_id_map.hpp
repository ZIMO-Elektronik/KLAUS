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
#include "app-window.h"
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
static MessageIDAdapter message_id_map(type::MessageID id) {
  using type::MessageID;

  switch (id) {
    case MessageID::Start: return MessageIDAdapter::Start;
    case MessageID::StartComplete: return MessageIDAdapter::StartComplete;
    case MessageID::StartError: return MessageIDAdapter::Default;
    case MessageID::Init: return MessageIDAdapter::Init;
    case MessageID::InitComplete: return MessageIDAdapter::InitComplete;
    case MessageID::SearchDecoder: return MessageIDAdapter::SearchDecoder;
    case MessageID::FoundDecoder: return MessageIDAdapter::FoundDecoder;
    case MessageID::EraseFlash: return MessageIDAdapter::EraseFlash;
    case MessageID::EraseFlashComplete:
      return MessageIDAdapter::EraseFlashComplete;
    case MessageID::WriteFlash: return MessageIDAdapter::WriteFlash;
    case MessageID::WriteFlashComplete:
      return MessageIDAdapter::WriteFlashComplete;
    case MessageID::ReadCv: return MessageIDAdapter::ReadCv;
    case MessageID::ReadCvComplete: return MessageIDAdapter::ReadCvComplete;
    case MessageID::WriteCv: return MessageIDAdapter::WriteCv;
    case MessageID::WriteCvComplete: return MessageIDAdapter::WriteCvComplete;
    case MessageID::Verify: return MessageIDAdapter::Verify;
    case MessageID::VerifyComplete: return MessageIDAdapter::VerifyComplete;
    case MessageID::VerifyError: return MessageIDAdapter::VerifyError;
    case MessageID::Cleanup: return MessageIDAdapter::Cleanup;
    case MessageID::CleanupComplete: return MessageIDAdapter::CleanupComplete;
    case MessageID::Done: return MessageIDAdapter::Done;
    case MessageID::Abort: return MessageIDAdapter::Abort;
    case MessageID::AbortInit: return MessageIDAdapter::AbortInit;
    case MessageID::AbortDecoderSearch:
      return MessageIDAdapter::AbortDecoderSearch;
    case MessageID::AbortFlashErase: return MessageIDAdapter::AbortFlashErase;
    case MessageID::AbortFlashWrite: return MessageIDAdapter::AbortFlashWrite;
    case MessageID::AbortVerify: return MessageIDAdapter::AbortVerify;
    default: return MessageIDAdapter::Default;
  }
}

} // namespace ui::helper
