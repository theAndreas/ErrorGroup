// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The second of two groups for SelfCheck, whose codes overlap on purpose.

#ifndef ERRORGROUP_EXAMPLES_SELFCHECK_BETA_ERROR_H_
#define ERRORGROUP_EXAMPLES_SELFCHECK_BETA_ERROR_H_

#include <stdint.h>
#include <utl/error_group.h>

namespace self_check {

enum class BetaError : uint8_t {
  kFirst,
};

class BetaErrorGroup : public utl::ErrorGroup {
 public:
  constexpr BetaErrorGroup() = default;

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

extern const BetaErrorGroup kBetaErrorGroup;

inline const utl::ErrorGroup& GetErrorGroup(BetaError) {
  return kBetaErrorGroup;
}

}  // namespace self_check

#endif  // ERRORGROUP_EXAMPLES_SELFCHECK_BETA_ERROR_H_
