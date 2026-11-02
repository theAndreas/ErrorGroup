// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// An error: the group it comes from and a code within that group.

#ifndef ERRORGROUP_SRC_UTL_ERROR_H_
#define ERRORGROUP_SRC_UTL_ERROR_H_

#include <Arduino.h>
#include <stdint.h>

#include "utl/error_group.h"

namespace utl {

// An error, 3 bytes on the Uno, or none: a default-constructed Error means "no
// error", and it is the only one whose group is null, so no group has to keep
// a code free for it.
//
//   Error SetChar(char c) {
//     if (c < ' ') {
//       return FontError::kUnknownChar;
//     }
//     return Error();
//   }
//
//   if (SetChar(c) == FontError::kUnknownChar) { ... }
//
// Two errors are equal if both the group and the code are, so codes of
// different groups may overlap. The fluent functions follow Rust's Result<(),
// E>: AndThen() goes on while there is no error, OrElse() recovers from one.
class Error {
 public:
  // No error.
  constexpr Error() : group_(nullptr), code_(0) {}

  // `code` of `group`.
  constexpr Error(const ErrorGroup& group, uint8_t code)
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

  bool IsOk() const { return group_ == nullptr; }

  // Null if IsOk().
  const ErrorGroup* group() const { return group_; }
  uint8_t code() const { return code_; }

  // Returns the text of the error, or "ok", from flash.
  const __FlashStringHelper* Message() const;

  // Returns `f()` if there is no error, or else this error; `f` is not
  // called then. `f` returns an Error or a Result.
  template <typename F>
  auto AndThen(F f) const -> decltype(f()) {
    if (!IsOk()) {
      return *this;
    }
    return f();
  }

  // Returns this error if there is none, or else `f(*this)`: `f` gets the
  // error and returns another, or Error() if it recovered.
  template <typename F>
  Error OrElse(F f) const {
    if (IsOk()) {
      return *this;
    }
    return f(*this);
  }

  // Hidden friends: found only through an Error, and with the implicit
  // conversion above on either side.
  friend bool operator==(const Error& a, const Error& b) {
    return a.group_ == b.group_ && a.code_ == b.code_;
  }
  friend bool operator!=(const Error& a, const Error& b) { return !(a == b); }

 private:
  const ErrorGroup* group_;
  uint8_t code_;
};

}  // namespace utl

#endif  // ERRORGROUP_SRC_UTL_ERROR_H_
