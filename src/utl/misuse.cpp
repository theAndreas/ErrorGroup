// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/misuse.h"

#include <avr/pgmspace.h>
#include <stddef.h>
#include <stdint.h>

#include "utl/array.h"

namespace utl {
namespace {

const char kResultValueText[] PROGMEM = "value() of a Result with an error";
const char kOptionalValueText[] PROGMEM = "* or -> of an empty optional";
const char kIndexText[] PROGMEM = "[] beyond the end";
const char kEmptyText[] PROGMEM = "front() or back() of an empty view";

// In the order of Misuse.
constexpr auto kTexts PROGMEM =
    make_array(kResultValueText, kOptionalValueText, kIndexText, kEmptyText);

}  // namespace

size_t PrintMisuse(Print& out, Misuse misuse) {
  const auto index = static_cast<uint8_t>(misuse);
  size_t count = out.print(F("utl: "));
  if (index < kTexts.size()) {
    count += out.print(
        static_cast<const __FlashStringHelper*>(pgm_read_ptr(&kTexts[index])));
  }
  return count;
}

}  // namespace utl
