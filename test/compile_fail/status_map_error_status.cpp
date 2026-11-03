// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Status::MapError() with a function that returns a Status, which could be ok:
// recovering is what OrElse() is for.
//
// Expected: MapError: f returns an Error or a code

#include <utl/status.h>

void Case() {
  static_cast<void>(utl::Status::Ok().MapError(
      [](utl::Error const&) { return utl::Status::Ok(); }));
}
