// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bus_error.h"

#include <avr/pgmspace.h>
#include <utl/array.h>
#include <utl/error_group.h>
#include <utl/messages.h>

namespace bus {
namespace {

char const kName[] PROGMEM = "bus";
char const kNackText[] PROGMEM = "nack";
char const kTimeoutText[] PROGMEM = "timeout";

// In the order of BusError.
constexpr auto kMessages PROGMEM = utl::make_array(kNackText, kTimeoutText);

}  // namespace

constexpr utl::ErrorGroup kBusErrorGroup{kName};

void UseBusMessages() {
  static utl::Messages messages{kBusErrorGroup, kMessages};
  utl::UseMessages(messages);
}

}  // namespace bus
