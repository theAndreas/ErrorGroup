// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A function_ref of a function given as a constant that is null: known when
// compiled, so it stops the build rather than being reported.
//
// Expected: function_ref: the function is null

#include <utl/constant_wrapper.h>
#include <utl/function_ref.h>

struct Led {
  void Toggle() {}
};

constexpr void (Led::*kNone)() = nullptr;

void Case() {
  Led led;
  utl::function_ref<void()> const f{UTL_CW(kNone), led};
  static_cast<void>(f);
}
