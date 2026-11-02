// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/error_group.h"

#include <avr/pgmspace.h>
#include <stddef.h>
#include <stdint.h>

namespace utl {

const __FlashStringHelper* ErrorGroup::LookUpMessage(
    const char* const* messages, uint8_t count, uint8_t code) {
  if (code >= count) {
    return F("unknown code");
  }
  return static_cast<const __FlashStringHelper*>(pgm_read_ptr(&messages[code]));
}

const __FlashStringHelper* ErrorGroup::NoMessage() { return F(""); }

}  // namespace utl
