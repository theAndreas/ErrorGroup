// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/optional_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>

#include "utl/array.h"

namespace utl {
namespace {

const char kEmptyText[] PROGMEM = "empty";

// In the order of OptionalError.
constexpr auto kMessages PROGMEM = make_array(kEmptyText);

}  // namespace

constexpr OptionalErrorGroup kOptionalErrorGroup{};

const __FlashStringHelper* OptionalErrorGroup::Name() const {
  return F("optional");
}

const __FlashStringHelper* OptionalErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace utl
