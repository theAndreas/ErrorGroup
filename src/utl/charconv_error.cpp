// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/charconv_error.h"

#include <avr/pgmspace.h>

#include "utl/array.h"
#include "utl/messages.h"

namespace utl {

char const kCharconvErrorGroup[] PROGMEM = "charconv";

namespace {

char const kInvalidArgumentText[] PROGMEM = "invalid argument";
char const kOutOfRangeText[] PROGMEM = "out of range";
char const kValueTooLargeText[] PROGMEM = "value too large";

// In the order of CharconvError.
constexpr auto kMessages PROGMEM =
    make_array(kInvalidArgumentText, kOutOfRangeText, kValueTooLargeText);

}  // namespace

void UseCharconvMessages() {
  // The group by its name, a constant: a static whose initializer calls a
  // function would be initialized at run time, with a guard.
  static Messages messages{ErrorGroup(kCharconvErrorGroup), kMessages};
  UseMessages(messages);
}

}  // namespace utl
