// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "alpha_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>
#include <utl/array.h>
#include <utl/error.h>

namespace self_check {
namespace {

char const kFirstText[] PROGMEM = "alpha first";
char const kSecondText[] PROGMEM = "alpha second";

// In the order of AlphaError.
constexpr auto kMessages PROGMEM = utl::make_array(kFirstText, kSecondText);

}  // namespace

constexpr AlphaErrorGroup kAlphaErrorGroup{};

__FlashStringHelper const* AlphaErrorGroup::Name() const { return F("alpha"); }

__FlashStringHelper const* AlphaErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

utl::Error MakeAlphaSecondElsewhere() { return AlphaError::kSecond; }

}  // namespace self_check
