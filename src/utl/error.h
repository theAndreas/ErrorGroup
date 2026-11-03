// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// An error: the group it comes from and a code within that group.

#ifndef UTL_SRC_UTL_ERROR_H_
#define UTL_SRC_UTL_ERROR_H_

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

class Status;

// An error, always: 3 bytes on the Uno, a group and a code within it. What a
// function returns that may succeed is a Status or a Result, which hold an
// Error only when something went wrong; "no error" is neither an Error nor
// a code a group would have to keep free.
//
//   const Error error = FontError::kUnknownChar;
//   error.group().Name();  // "font", never null
//   error.Message();       // "unknown char"
//
// Two errors are equal if both the group and the code are, so codes of
// different groups may overlap.
class [[nodiscard]] Error {
 public:
  // `code` of `group`.
  constexpr Error(ErrorGroup const& group, uint8_t code)
      : group_(&group), code_(code) {}

  // A code of a group, found through GetErrorGroup(code). Implicit, unlike the
  // Google default, because a code of a group is an error and nothing else:
  // `return FontError::kUnknownChar;` and `error == FontError::kUnknownChar`
  // say what they mean, with no function call around the code. Takes part in
  // overload resolution only for an enum with a GetErrorGroup(), so no other
  // type converts.
  template <typename Code, typename = decltype(GetErrorGroup(Code()))>
  constexpr Error(Code code)  // NOLINT(runtime/explicit)
      : group_(&GetErrorGroup(code)), code_(static_cast<uint8_t>(code)) {}

  ErrorGroup const& group() const { return *group_; }
  uint8_t code() const { return code_; }

  // Returns the text of the error, from flash; an empty text if
  // UTL_ERROR_TEXTS is 0.
  __FlashStringHelper const* Message() const;

  // Prints the group and the text, "bus: nack", or the group and the code,
  // "bus: 0", if UTL_ERROR_TEXTS is 0. Returns the number of chars, as
  // Print::print() does. Not Printable, whose vptr would make an Error 5
  // bytes rather than 3.
  size_t PrintTo(Print& out) const;

  // Hidden friends: found only through an Error, and with the implicit
  // conversion above on either side.
  friend bool operator==(Error const& a, Error const& b) {
    return a.group_ == b.group_ && a.code_ == b.code_;
  }
  friend bool operator!=(Error const& a, Error const& b) { return !(a == b); }

 private:
  friend class Status;

  // Status keeps an Error with a null group for "ok", so that it needs no
  // flag beside it and stays 3 bytes. Private: only Status makes one, and it
  // never hands it out.
  constexpr explicit Error(decltype(nullptr)) : group_(nullptr), code_(0) {}

  // A pointer, not a reference, so that an Error can be assigned. Never null
  // outside a Status.
  ErrorGroup const* group_;
  uint8_t code_;
};

}  // namespace utl

#endif  // UTL_SRC_UTL_ERROR_H_
