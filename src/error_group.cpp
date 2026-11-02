// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "error_group.h"

#include <avr/pgmspace.h>

namespace error_group {

const __FlashStringHelper* ErrorGroup::LookUpMessage(
    const char* const* messages, uint8_t count, uint8_t code) {
  if (code >= count) {
    return F("unknown code");
  }
  return static_cast<const __FlashStringHelper*>(pgm_read_ptr(&messages[code]));
}

const __FlashStringHelper* Error::Message() const {
  if (IsOk()) {
    return F("ok");
  }
  return group_->Message(code_);
}

}  // namespace error_group
