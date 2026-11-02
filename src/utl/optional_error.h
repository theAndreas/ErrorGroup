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

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

// Defined in optional_error.cpp. A sketch that never calls value() of an
// optional does not reference it, and the linker leaves it out with its
// vtable.
extern const OptionalErrorGroup kOptionalErrorGroup;

inline const ErrorGroup& GetErrorGroup(OptionalError) {
  return kOptionalErrorGroup;
}

}  // namespace utl

#endif  // UTL_SRC_UTL_OPTIONAL_ERROR_H_
