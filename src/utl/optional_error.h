// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of an optional: value() of one that holds none.

#ifndef UTL_SRC_UTL_OPTIONAL_ERROR_H_
#define UTL_SRC_UTL_OPTIONAL_ERROR_H_

#include <Arduino.h>
#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

enum class OptionalError : uint8_t {
  kEmpty,
};

class OptionalErrorGroup : public ErrorGroup {
 public:
  constexpr OptionalErrorGroup() = default;

  __FlashStringHelper const* Name() const override;
  __FlashStringHelper const* Message(uint8_t code) const override;
};

// Defined in optional_error.cpp. A sketch that never calls value() of an
// optional does not reference it, and the linker leaves it out with its
// vtable.
extern OptionalErrorGroup const kOptionalErrorGroup;

inline ErrorGroup const& GetErrorGroup(OptionalError) {
  return kOptionalErrorGroup;
}

}  // namespace utl

#endif  // UTL_SRC_UTL_OPTIONAL_ERROR_H_
