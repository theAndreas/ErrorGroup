// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The report of an access that is a mistake, with UTL_CHECK_ACCESS at 1.

#ifndef UTL_SRC_UTL_MISUSE_H_
#define UTL_SRC_UTL_MISUSE_H_

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "utl/config.h"

namespace utl {

// What was done that should not have been. An enum rather than a text at the
// place of the check: the checks lie in templates, where F() would put one
// text into flash for every instantiation, and where avr-gcc can end in a
// "section type conflict".
enum class Misuse : uint8_t {
  kResultValue,    // value() of a Result with an error
  kOptionalValue,  // * or -> of an empty optional
  kIndex,          // [] beyond the end
  kEmpty,          // front() or back() of an empty span or string_view
};

// Takes the report of a misuse, with UTL_CHECK_ACCESS at 1, before the access
// happens; what follows is what the access does unchecked. The default prints
// one line on Serial, "utl: [] beyond the end". It is weak, so that a sketch
// whose Serial carries something else, or which wants to halt or blink
// instead, defines its own, and the default and Serial are left out.
void ReportMisuse(Misuse misuse);

// Prints "utl: [] beyond the end" on `out`, for a report of a sketch's own.
// Returns the number of chars.
size_t PrintMisuse(Print& out, Misuse misuse);

namespace internal {

// Reports `misuse` unless `ok`, and returns `ok`. constexpr, so that the
// accessors that call it stay constexpr: the report lies in the branch of
// ?: that a right access never takes, so a constant expression never reaches
// it, and with the switch off there is none.
#if UTL_CHECK_ACCESS
constexpr bool Check(bool ok, Misuse misuse) {
  return ok ? true : (ReportMisuse(misuse), false);
}
#else
constexpr bool Check(bool ok, Misuse /*misuse*/) { return ok; }
#endif

}  // namespace internal
}  // namespace utl

#endif  // UTL_SRC_UTL_MISUSE_H_
