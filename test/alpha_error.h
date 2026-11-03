// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A group of errors for the tests, whose codes overlap with the other's. Its
// texts are turned on by the tests that print it.

#ifndef UTL_TEST_ALPHA_ERROR_H_
#define UTL_TEST_ALPHA_ERROR_H_

#include <stdint.h>
#include <utl/error.h>
#include <utl/error_group.h>

namespace utl_test {

enum class AlphaError : uint8_t {
  kFirst,
  kSecond,
};

extern utl::ErrorGroup const kAlphaErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(AlphaError) {
  return kAlphaErrorGroup;
}

// Turns on the texts of the group.
void UseAlphaMessages();

// Made in alpha_error.cc, so that a test compares an error from another
// translation unit: equal only if both see the group at the same address.
utl::Error MakeAlphaSecondElsewhere();
}  // namespace utl_test

#endif  // UTL_TEST_ALPHA_ERROR_H_
