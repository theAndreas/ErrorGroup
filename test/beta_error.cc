// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "test/beta_error.h"

#include <avr/pgmspace.h>
#include <utl/error_group.h>

namespace utl_test {
namespace {

char const kName[] PROGMEM = "beta";

}  // namespace

constexpr utl::ErrorGroup kBetaErrorGroup{kName};

}  // namespace utl_test
