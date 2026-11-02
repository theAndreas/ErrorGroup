// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of a font, one of two groups this example defines outside the
// library.

#ifndef ERRORGROUP_EXAMPLES_BASICS_FONT_ERROR_H_
#define ERRORGROUP_EXAMPLES_BASICS_FONT_ERROR_H_

#include <error_group.h>

namespace font {

enum class FontError : uint8_t {
  kUnknownChar,
  kNullText,
};

class FontErrorGroup : public error_group::ErrorGroup {
 public:
  constexpr FontErrorGroup() = default;

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

// Defined in font_error.cpp, and nowhere else: its address is the group.
extern const FontErrorGroup kFontErrorGroup;

inline const error_group::ErrorGroup& GetErrorGroup(FontError) {
  return kFontErrorGroup;
}

}  // namespace font

#endif  // ERRORGROUP_EXAMPLES_BASICS_FONT_ERROR_H_
