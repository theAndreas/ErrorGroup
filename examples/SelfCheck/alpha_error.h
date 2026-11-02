// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The first of two groups for SelfCheck, whose codes overlap on purpose.

#ifndef ERRORGROUP_EXAMPLES_SELFCHECK_ALPHA_ERROR_H_
#define ERRORGROUP_EXAMPLES_SELFCHECK_ALPHA_ERROR_H_

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

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

extern const AlphaErrorGroup kAlphaErrorGroup;

inline const utl::ErrorGroup& GetErrorGroup(AlphaError) {
  return kAlphaErrorGroup;
}

// Made in alpha_error.cpp, so that SelfCheck compares an error from another
// translation unit: equal only if both see the group at the same address.
utl::Error MakeAlphaSecondElsewhere();

}  // namespace self_check

#endif  // ERRORGROUP_EXAMPLES_SELFCHECK_ALPHA_ERROR_H_
