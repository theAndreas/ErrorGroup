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

class AlphaErrorGroup : public utl::ErrorGroup {
 public:
  constexpr AlphaErrorGroup() = default;

  __FlashStringHelper const* Name() const override;
  __FlashStringHelper const* Message(uint8_t code) const override;
};

extern AlphaErrorGroup const kAlphaErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(AlphaError) {
  return kAlphaErrorGroup;
}

// Made in alpha_error.cpp, so that SelfCheck compares an error from another
// translation unit: equal only if both see the group at the same address.
utl::Error MakeAlphaSecondElsewhere();

}  // namespace self_check

#endif  // UTL_EXAMPLES_SELFCHECK_ALPHA_ERROR_H_
