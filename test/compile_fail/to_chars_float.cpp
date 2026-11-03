// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// to_chars() of a float, which is left out: a correct one would not fit on
// an Uno, and avr-libc has dtostrf().
//
// Expected: to_chars: T is an integer other than bool

#include <utl/charconv.h>

void Case() {
  char buffer[8];  // NOLINT(modernize-avoid-c-arrays)
  static_cast<void>(utl::to_chars(buffer, buffer + 8, 1.5F));
}
