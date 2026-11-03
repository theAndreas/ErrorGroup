// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "beta_error.h"

#include <avr/pgmspace.h>
#include <utl/array.h>
#include <utl/error_group.h>
#include <utl/messages.h>

namespace self_check {
namespace {

char const kName[] PROGMEM = "beta";
char const kFirstText[] PROGMEM = "beta first";

// In the order of BetaError.
constexpr auto kMessages PROGMEM = utl::make_array(kFirstText);

}  // namespace

constexpr utl::ErrorGroup kBetaErrorGroup{kName};

void UseBetaMessages() {
  static utl::Messages messages{kBetaErrorGroup, kMessages};
  utl::UseMessages(messages);
}

}  // namespace self_check
