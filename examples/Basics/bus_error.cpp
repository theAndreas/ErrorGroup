// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bus_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>
#include <utl/array.h>

namespace bus {
namespace {

char const kNackText[] PROGMEM = "nack";
char const kTimeoutText[] PROGMEM = "timeout";

// In the order of BusError.
constexpr auto kMessages PROGMEM = utl::make_array(kNackText, kTimeoutText);

}  // namespace

constexpr BusErrorGroup kBusErrorGroup{};

__FlashStringHelper const* BusErrorGroup::Name() const { return F("bus"); }

__FlashStringHelper const* BusErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace bus
