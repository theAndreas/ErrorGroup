// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of an index, the group of array and span.

#ifndef UTL_SRC_UTL_INDEX_ERROR_H_
#define UTL_SRC_UTL_INDEX_ERROR_H_

#include <avr/pgmspace.h>
#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

enum class IndexError : uint8_t {
  kOutOfRange,
};

// "index", defined in index_error.cpp; its address is the group. A
// sketch that never calls at(), first(), last() or subspan() does not
// reference it, and the linker leaves it out.
// A name in flash is a char array, declared here without its text, as only
// one file may define it.
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
extern char const kIndexErrorGroup[] PROGMEM;

constexpr ErrorGroup GetErrorGroup(IndexError) {
  return ErrorGroup(kIndexErrorGroup);
}

// Turns on the texts of the group, see messages.h.
void UseIndexMessages();

}  // namespace utl

#endif  // UTL_SRC_UTL_INDEX_ERROR_H_
