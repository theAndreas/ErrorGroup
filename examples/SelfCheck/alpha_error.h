// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The first of two groups for SelfCheck, whose codes overlap on purpose.

#ifndef UTL_EXAMPLES_SELFCHECK_ALPHA_ERROR_H_
#define UTL_EXAMPLES_SELFCHECK_ALPHA_ERROR_H_

#include <stdint.h>
#include <utl/error.h>
#include <utl/error_group.h>

namespace self_check {

enum class AlphaError : uint8_t {
  kFirst,
  kSecond,
};

extern utl::ErrorGroup const kAlphaErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(AlphaError) {
  return kAlphaErrorGroup;
}

// Turns on the texts of the group: until then its errors print their codes.
void UseAlphaMessages();

// Made in alpha_error.cpp, so that SelfCheck compares an error from another
// translation unit: equal only if both see the group at the same address.
utl::Error MakeAlphaSecondElsewhere();

}  // namespace self_check

#endif  // UTL_EXAMPLES_SELFCHECK_ALPHA_ERROR_H_
