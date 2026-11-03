// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Status::InspectError() with a function that returns a Status, which would be
// dropped unseen.
//
// Expected: InspectError: f returns nothing

#include <utl/status.h>

void Case() {
  static_cast<void>(utl::Status::Ok().InspectError(
      [](utl::Error const&) { return utl::Status::Ok(); }));
}
