// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "font_error.h"

#include <avr/pgmspace.h>
#include <utl/array.h>
#include <utl/error_group.h>
#include <utl/messages.h>

namespace font {
namespace {

char const kName[] PROGMEM = "font";
char const kUnknownCharText[] PROGMEM = "unknown char";
char const kNullTextText[] PROGMEM = "null text";

// In the order of FontError.
constexpr auto kMessages PROGMEM =
    utl::make_array(kUnknownCharText, kNullTextText);

}  // namespace

constexpr utl::ErrorGroup kFontErrorGroup{kName};

void UseFontMessages() {
  static utl::Messages messages{kFontErrorGroup, kMessages};
  utl::UseMessages(messages);
}

}  // namespace font
