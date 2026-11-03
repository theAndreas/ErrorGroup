// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of an optional: value() of one that holds none.

#ifndef UTL_SRC_UTL_OPTIONAL_ERROR_H_
#define UTL_SRC_UTL_OPTIONAL_ERROR_H_

#include <avr/pgmspace.h>
#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

enum class OptionalError : uint8_t {
  kEmpty,
};

// "optional", defined in optional_error.cpp; its address is the group. A
// sketch that never calls value() of an optional does not reference it, and
// the linker leaves it out.
// A name in flash is a char array, declared here without its text, as only
// one file may define it.
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
extern char const kOptionalErrorGroup[] PROGMEM;

constexpr ErrorGroup GetErrorGroup(OptionalError) {
  return ErrorGroup(kOptionalErrorGroup);
}

// Turns on the texts of the group, see messages.h.
void UseOptionalMessages();

}  // namespace utl

#endif  // UTL_SRC_UTL_OPTIONAL_ERROR_H_
