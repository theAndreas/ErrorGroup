// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test/alpha_error.h"

#include <avr/pgmspace.h>
#include <utl/array.h>
#include <utl/error.h>
#include <utl/error_group.h>
#include <utl/messages.h>

namespace utl_test {
namespace {

char const kName[] PROGMEM = "alpha";
char const kFirstText[] PROGMEM = "alpha first";
char const kSecondText[] PROGMEM = "alpha second";

constexpr auto kMessages PROGMEM = utl::make_array(kFirstText, kSecondText);

}  // namespace

constexpr utl::ErrorGroup kAlphaErrorGroup{kName};

void UseAlphaMessages() {
  static utl::Messages messages{kAlphaErrorGroup, kMessages};
  utl::UseMessages(messages);
}

utl::Error MakeAlphaSecondElsewhere() { return AlphaError::kSecond; }
}  // namespace utl_test
