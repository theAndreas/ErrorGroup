// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Status::AndThen() with a function that returns a plain value, which Map() is
// for.
//
// Expected: AndThen: f returns a Status or a Result

#include <utl/status.h>

void Case() {
  static_cast<void>(utl::Status::Ok().AndThen([] { return 1; }));
}
