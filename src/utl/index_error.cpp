// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/index_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>

#include "utl/array.h"

namespace utl {
namespace {

char const kOutOfRangeText[] PROGMEM = "out of range";

// In the order of IndexError.
constexpr auto kMessages PROGMEM = make_array(kOutOfRangeText);

}  // namespace

constexpr IndexErrorGroup kIndexErrorGroup{};

__FlashStringHelper const* IndexErrorGroup::Name() const { return F("index"); }

__FlashStringHelper const* IndexErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace utl
