// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test/beta_error.h"

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <utl/array.h>
namespace utl_test {
namespace {

char const kFirstText[] PROGMEM = "beta first";

constexpr auto kMessages PROGMEM = utl::make_array(kFirstText);

}  // namespace

constexpr BetaErrorGroup kBetaErrorGroup{};

__FlashStringHelper const* BetaErrorGroup::Name() const { return F("beta"); }

__FlashStringHelper const* BetaErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace utl_test
