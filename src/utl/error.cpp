// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/error.h"

namespace utl {

const __FlashStringHelper* Error::Message() const {
  if (IsOk()) {
    return F("ok");
  }
  return group_->Message(code_);
}

}  // namespace utl
