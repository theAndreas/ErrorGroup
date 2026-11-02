// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// What a function returns that has nothing else to return: ok, or an error.

#ifndef ERRORGROUP_SRC_UTL_STATUS_H_
#define ERRORGROUP_SRC_UTL_STATUS_H_

#include <Arduino.h>

#include "utl/error.h"

namespace utl {

// Ok, or an Error: 3 bytes on the Uno, as Rust's Result<(), E> or
// absl::Status.
//
//   Status SetChar(char c) {
//     if (c < ' ') {
//       return FontError::kUnknownChar;
//     }
//     return Status::Ok();
//   }
//
//   const Status status = SetChar(c);
//   if (status == FontError::kUnknownChar) { ... }
//   if (const Error* error = status.error()) {
//     error->group().Name();
//   }
//
// The fluent functions follow Rust: AndThen() goes on while there is no
// error, OrElse() recovers from one.
class Status {
 public:
  // Ok. Status::Ok() says the same where it is returned.
  constexpr Status() : error_(nullptr) {}
  static constexpr Status Ok() { return Status(); }

  // An error. Implicit, as a function that returns a Status returns its error
  // as it is.
  constexpr Status(const Error& error)  // NOLINT(runtime/explicit)
      : error_(error) {}

  // A code of a group, as in the implicit constructor of Error. Without it,
  // `return FontError::kUnknownChar;` would need two conversions in a row,
  // which C++ does not make implicitly.
  template <typename Code, typename = decltype(GetErrorGroup(Code()))>
  constexpr Status(Code code)  // NOLINT(runtime/explicit)
      : error_(code) {}

  bool IsOk() const { return error_.group_ == nullptr; }

  // The error, or null if IsOk(). A pointer, so that no one gets hold of an
  // Error that is none.
  const Error* error() const { return IsOk() ? nullptr : &error_; }

  // Returns the text of the error, or "ok", from flash.
  const __FlashStringHelper* Message() const;

  // Returns `f()` if there is no error, or else the error; `f` is not called
  // then. `f` returns a Status or a Result.
  template <typename F>
  auto AndThen(F f) const -> decltype(f()) {
    if (!IsOk()) {
      return error_;
    }
    return f();
  }

  // Returns this if there is no error, or else `f(error)`: `f` gets an Error
  // that is one, and returns another, or Status::Ok() if it recovered.
  template <typename F>
  Status OrElse(F f) const {
    if (IsOk()) {
      return *this;
    }
    return f(error_);
  }

  // Hidden friends, with the conversions above on either side, so that a
  // Status compares with another, an Error or a code. Two ok ones are equal,
  // as both hold the same null group and code 0.
  friend bool operator==(const Status& a, const Status& b) {
    return a.error_ == b.error_;
  }
  friend bool operator!=(const Status& a, const Status& b) { return !(a == b); }

 private:
  // With a null group if ok.
  Error error_;
};

}  // namespace utl

#endif  // ERRORGROUP_SRC_UTL_STATUS_H_
