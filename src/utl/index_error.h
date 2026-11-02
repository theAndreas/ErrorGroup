// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of an index, the group of array and span.

#ifndef UTL_SRC_UTL_INDEX_ERROR_H_
#define UTL_SRC_UTL_INDEX_ERROR_H_

#include <Arduino.h>
#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

enum class IndexError : uint8_t {
  kOutOfRange,
};

class IndexErrorGroup : public ErrorGroup {
 public:
  constexpr IndexErrorGroup() = default;

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

// Defined in index_error.cpp. A sketch that never calls at(), first(), last()
// or subspan() does not reference it, and the linker leaves it out with its
// vtable.
extern const IndexErrorGroup kIndexErrorGroup;

inline const ErrorGroup& GetErrorGroup(IndexError) { return kIndexErrorGroup; }

}  // namespace utl

#endif  // UTL_SRC_UTL_INDEX_ERROR_H_
