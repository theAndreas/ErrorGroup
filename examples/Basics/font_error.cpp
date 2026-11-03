// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "font_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>
#include <utl/array.h>

namespace font {
namespace {

char const kUnknownCharText[] PROGMEM = "unknown char";
char const kNullTextText[] PROGMEM = "null text";

// In the order of FontError.
constexpr auto kMessages PROGMEM =
    utl::make_array(kUnknownCharText, kNullTextText);

}  // namespace

constexpr FontErrorGroup kFontErrorGroup{};

__FlashStringHelper const* FontErrorGroup::Name() const { return F("font"); }

__FlashStringHelper const* FontErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace font
