// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The second of two groups for SelfCheck, whose codes overlap on purpose.

#ifndef UTL_EXAMPLES_SELFCHECK_BETA_ERROR_H_
#define UTL_EXAMPLES_SELFCHECK_BETA_ERROR_H_

#include <stdint.h>
#include <utl/error_group.h>

namespace self_check {

enum class BetaError : uint8_t {
  kFirst,
};

class BetaErrorGroup : public utl::ErrorGroup {
 public:
  constexpr BetaErrorGroup() = default;

  __FlashStringHelper const* Name() const override;
  __FlashStringHelper const* Message(uint8_t code) const override;
};

extern BetaErrorGroup const kBetaErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(BetaError) {
  return kBetaErrorGroup;
}

}  // namespace self_check

#endif  // UTL_EXAMPLES_SELFCHECK_BETA_ERROR_H_
