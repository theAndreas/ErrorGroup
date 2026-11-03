// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/optional_error.h"

#include <avr/pgmspace.h>

#include "utl/array.h"
#include "utl/error_group.h"
#include "utl/messages.h"

namespace utl {
namespace {

char const kName[] PROGMEM = "optional";
char const kEmptyText[] PROGMEM = "empty";

// In the order of OptionalError.
constexpr auto kMessages PROGMEM = make_array(kEmptyText);

}  // namespace

constexpr ErrorGroup kOptionalErrorGroup{kName};

void UseOptionalMessages() {
  static Messages messages{kOptionalErrorGroup, kMessages};
  UseMessages(messages);
}

}  // namespace utl
