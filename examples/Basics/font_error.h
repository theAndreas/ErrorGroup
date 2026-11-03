// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of a font, one of two groups this example defines outside the
// library.

#ifndef UTL_EXAMPLES_BASICS_FONT_ERROR_H_
#define UTL_EXAMPLES_BASICS_FONT_ERROR_H_

#include <stdint.h>
#include <utl/error_group.h>

namespace font {

enum class FontError : uint8_t {
  kUnknownChar,
  kNullText,
};

// Defined in font_error.cpp, and nowhere else: its address is the group.
extern utl::ErrorGroup const kFontErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(FontError) {
  return kFontErrorGroup;
}

// Turns on the texts of the group: until then its errors print their codes.
void UseFontMessages();

}  // namespace font

#endif  // UTL_EXAMPLES_BASICS_FONT_ERROR_H_
