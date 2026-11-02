// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test_error.h"

#include <avr/pgmspace.h>

namespace self_check {
namespace {

const char kAlphaFirstText[] PROGMEM = "alpha first";
const char kAlphaSecondText[] PROGMEM = "alpha second";
const char kBetaFirstText[] PROGMEM = "beta first";

// In the order of AlphaError.
const char* const kAlphaMessages[] PROGMEM = {
    kAlphaFirstText,
    kAlphaSecondText,
};

// In the order of BetaError.
const char* const kBetaMessages[] PROGMEM = {
    kBetaFirstText,
};

}  // namespace

constexpr AlphaErrorGroup kAlphaErrorGroup{};
constexpr BetaErrorGroup kBetaErrorGroup{};

const __FlashStringHelper* AlphaErrorGroup::Name() const { return F("alpha"); }

const __FlashStringHelper* AlphaErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(
      kAlphaMessages, sizeof(kAlphaMessages) / sizeof(kAlphaMessages[0]), code);
}

const __FlashStringHelper* BetaErrorGroup::Name() const { return F("beta"); }

const __FlashStringHelper* BetaErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kBetaMessages,
                       sizeof(kBetaMessages) / sizeof(kBetaMessages[0]), code);
}

error_group::Error MakeAlphaSecondElsewhere() {
  return error_group::MakeError(AlphaError::kSecond);
}

}  // namespace self_check
