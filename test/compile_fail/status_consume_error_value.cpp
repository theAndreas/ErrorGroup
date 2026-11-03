// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Status::ConsumeError() with a function that returns a value, which would be
// dropped unseen.
//
// Expected: ConsumeError: f returns nothing

#include <utl/status.h>

void Case() {
  utl::Status::Ok().ConsumeError([](utl::Error const&) { return 1; });
}
