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

extern utl::ErrorGroup const kBetaErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(BetaError) {
  return kBetaErrorGroup;
}

// Turns on the texts of the group: until then its errors print their codes.
void UseBetaMessages();

}  // namespace self_check

#endif  // UTL_EXAMPLES_SELFCHECK_BETA_ERROR_H_
