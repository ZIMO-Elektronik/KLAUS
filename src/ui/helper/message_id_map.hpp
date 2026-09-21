/**
 * Copyright (C) 2026 [ZIMO Elektronik]
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
 * MessageID to string converter
 *
 * \file    ui/helper/message_id_to_string.hpp
 * \author  Jonas Gahlert
 * \date    24.06.2026
 */

#pragma once

#include <string_view>
#include "app-window.h"
#include "type/step.hpp"

namespace ui::helper {

/**
 * Converts the given MessageID to its corresponding Message
 *
 * \note
 * Because of the problems with gettext, this may become obsolete in favor of a
 * fluent-based translation system
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
    case MessageID::AbortUnresponsive:
      return MessageIDAdapter::AbortUnresponsive;
    default: return MessageIDAdapter::Default;
  }
}

} // namespace ui::helper
