// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of to_chars() and from_chars(), the group of charconv.h.

#ifndef UTL_SRC_UTL_CHARCONV_ERROR_H_
#define UTL_SRC_UTL_CHARCONV_ERROR_H_

#include <avr/pgmspace.h>
#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

// The three values of std::errc that std::to_chars() and std::from_chars()
// report, under their names.
enum class CharconvError : uint8_t {
  kInvalidArgument,  // no number where one should begin, or a base not 2..36
  kOutOfRange,       // a number too large or too small for its type
  kValueTooLarge,    // the text of the number does not fit into the buffer
};

// "charconv", defined in charconv_error.cpp; its address is the group. A
// sketch that never converts does not reference it, and the linker leaves it
// out.
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
extern char const kCharconvErrorGroup[] PROGMEM;

constexpr ErrorGroup GetErrorGroup(CharconvError) {
  return ErrorGroup(kCharconvErrorGroup);
}

// Turns on the texts of the group, see messages.h.
void UseCharconvMessages();

}  // namespace utl

#endif  // UTL_SRC_UTL_CHARCONV_ERROR_H_
