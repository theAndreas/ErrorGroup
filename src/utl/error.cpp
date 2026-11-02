// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utl/error.h"

namespace utl {

const __FlashStringHelper* Error::Message() const {
  return group_->Message(code_);
}

}  // namespace utl
