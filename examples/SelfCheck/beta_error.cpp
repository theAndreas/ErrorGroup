// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "beta_error.h"

#include <avr/pgmspace.h>

namespace self_check {
namespace {

const char kFirstText[] PROGMEM = "beta first";

// In the order of BetaError.
const char* const kMessages[] PROGMEM = {
    kFirstText,
};

}  // namespace

constexpr BetaErrorGroup kBetaErrorGroup{};

const __FlashStringHelper* BetaErrorGroup::Name() const { return F("beta"); }

const __FlashStringHelper* BetaErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, sizeof(kMessages) / sizeof(kMessages[0]),
                       code);
}

}  // namespace self_check
