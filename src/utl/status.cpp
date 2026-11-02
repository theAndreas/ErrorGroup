// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/status.h"

#include <stddef.h>

namespace utl {

const __FlashStringHelper* Status::Message() const {
  if (IsOk()) {
    return F("ok");
  }
  return error_.Message();
}

size_t Status::PrintTo(Print& out) const {
  if (IsOk()) {
    return out.print(Message());
  }
  return error_.PrintTo(out);
}

}  // namespace utl
