// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/index_error.h"

#include <avr/pgmspace.h>

#include "utl/array.h"
#include "utl/error_group.h"
#include "utl/messages.h"

namespace utl {
namespace {

char const kName[] PROGMEM = "index";
char const kOutOfRangeText[] PROGMEM = "out of range";

// In the order of IndexError.
constexpr auto kMessages PROGMEM = make_array(kOutOfRangeText);

}  // namespace

constexpr ErrorGroup kIndexErrorGroup{kName};

void UseIndexMessages() {
  static Messages messages{kIndexErrorGroup, kMessages};
  UseMessages(messages);
}

}  // namespace utl
