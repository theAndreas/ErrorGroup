// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A group of errors for the tests, whose codes overlap with the other's.

#ifndef UTL_TEST_BETA_ERROR_H_
#define UTL_TEST_BETA_ERROR_H_

#include <Arduino.h>
#include <stdint.h>
#include <utl/error.h>
#include <utl/error_group.h>

namespace utl_test {

enum class BetaError : uint8_t {
  kFirst,
};

class BetaErrorGroup : public utl::ErrorGroup {
 public:
  constexpr BetaErrorGroup() = default;

  [[nodiscard]] __FlashStringHelper const* Name() const override;
  [[nodiscard]] __FlashStringHelper const* Message(uint8_t code) const override;
};

extern BetaErrorGroup const kBetaErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(BetaError) {
  return kBetaErrorGroup;
}

}  // namespace utl_test

#endif  // UTL_TEST_BETA_ERROR_H_
