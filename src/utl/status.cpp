// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/status.h"

namespace utl {

const __FlashStringHelper* Status::Message() const {
  if (IsOk()) {
    return F("ok");
  }
  return error_.Message();
}

}  // namespace utl
