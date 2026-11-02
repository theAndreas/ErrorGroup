// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bus_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>

namespace bus {
namespace {

const char kNackText[] PROGMEM = "nack";
const char kTimeoutText[] PROGMEM = "timeout";

// In the order of BusError.
const char* const kMessages[] PROGMEM = {
    kNackText,
    kTimeoutText,
};

}  // namespace

constexpr BusErrorGroup kBusErrorGroup{};

const __FlashStringHelper* BusErrorGroup::Name() const { return F("bus"); }

const __FlashStringHelper* BusErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, sizeof(kMessages) / sizeof(kMessages[0]),
                       code);
}

}  // namespace bus
