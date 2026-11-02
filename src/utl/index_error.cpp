// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/index_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>

#include "utl/array.h"

namespace utl {
namespace {

const char kOutOfRangeText[] PROGMEM = "out of range";

// In the order of IndexError.
const array<const char*, 1> kMessages PROGMEM = {
    kOutOfRangeText,
};

}  // namespace

constexpr IndexErrorGroup kIndexErrorGroup{};

const __FlashStringHelper* IndexErrorGroup::Name() const { return F("index"); }

const __FlashStringHelper* IndexErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace utl
