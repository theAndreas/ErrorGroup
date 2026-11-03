// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/index_error.h"

#include <avr/pgmspace.h>

#include "utl/array.h"
#include "utl/messages.h"

namespace utl {

char const kIndexErrorGroup[] PROGMEM = "index";

namespace {

char const kOutOfRangeText[] PROGMEM = "out of range";

// In the order of IndexError.
constexpr auto kMessages PROGMEM = make_array(kOutOfRangeText);

}  // namespace

void UseIndexMessages() {
  // The group by its name, a constant: a static whose initializer calls a
  // function would be initialized at run time, with a guard.
  static Messages messages{ErrorGroup(kIndexErrorGroup), kMessages};
  UseMessages(messages);
}

}  // namespace utl
