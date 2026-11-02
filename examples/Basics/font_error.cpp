// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "font_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>

namespace font {
namespace {

const char kUnknownCharText[] PROGMEM = "unknown char";
const char kNullTextText[] PROGMEM = "null text";

// In the order of FontError.
const char* const kMessages[] PROGMEM = {
    kUnknownCharText,
    kNullTextText,
};

}  // namespace

constexpr FontErrorGroup kFontErrorGroup{};

const __FlashStringHelper* FontErrorGroup::Name() const { return F("font"); }

const __FlashStringHelper* FontErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, sizeof(kMessages) / sizeof(kMessages[0]),
                       code);
}

}  // namespace font
