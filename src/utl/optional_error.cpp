// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/optional_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>

#include "utl/array.h"

namespace utl {
namespace {

char const kEmptyText[] PROGMEM = "empty";

// In the order of OptionalError.
constexpr auto kMessages PROGMEM = make_array(kEmptyText);

}  // namespace

constexpr OptionalErrorGroup kOptionalErrorGroup{};

__FlashStringHelper const* OptionalErrorGroup::Name() const {
  return F("optional");
}

__FlashStringHelper const* OptionalErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace utl
