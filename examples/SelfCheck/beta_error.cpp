// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "beta_error.h"

#include <avr/pgmspace.h>
#include <stdint.h>
#include <utl/array.h>

namespace self_check {
namespace {

const char kFirstText[] PROGMEM = "beta first";

// In the order of BetaError.
const utl::array<const char*, 1> kMessages PROGMEM = {
    kFirstText,
};

}  // namespace

constexpr BetaErrorGroup kBetaErrorGroup{};

const __FlashStringHelper* BetaErrorGroup::Name() const { return F("beta"); }

const __FlashStringHelper* BetaErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);
}

}  // namespace self_check
