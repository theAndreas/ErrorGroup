// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of an index, the group of array and span.

#ifndef UTL_SRC_UTL_INDEX_ERROR_H_
#define UTL_SRC_UTL_INDEX_ERROR_H_

#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

enum class IndexError : uint8_t {
  kOutOfRange,
};

// Defined in index_error.cpp. A sketch that never calls at(), first(), last()
// or subspan() does not reference it, and the linker leaves it out.
extern ErrorGroup const kIndexErrorGroup;

inline ErrorGroup const& GetErrorGroup(IndexError) { return kIndexErrorGroup; }

// Turns on the texts of the group, see messages.h.
void UseIndexMessages();

}  // namespace utl

#endif  // UTL_SRC_UTL_INDEX_ERROR_H_
