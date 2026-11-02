// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A value or the error that kept a function from returning one.

#ifndef ERRORGROUP_SRC_UTL_RESULT_H_
#define ERRORGROUP_SRC_UTL_RESULT_H_

#include "utl/error.h"
#include "utl/internal/type_traits.h"
#include "utl/status.h"

namespace utl {

// A value of T, or an Error. The value and a Status lie side by side, not in a
// union: T needs a default constructor, for the value of a Result with an
// error, and in return the class needs no tag, no placement new and no
// deviation under a safety standard's rules on unions.
//
//   Result<uint8_t> GetRow(uint8_t index) {
//     if (index >= kRows) {
//       return PanelError::kOutOfRange;
//     }
//     return rows[index];
//   }
//
//   const bool lit = GetRow(3)
//       .Map([](uint8_t row) { return row != 0; })
//       .ValueOr(false);
//
// The fluent functions follow Rust's Result: AndThen() goes on with the value,
// Map() changes it, OrElse() recovers from the error. A function passed to
// them returns by value; none of them calls it when it has nothing to give.
template <typename T>
class Result {
 public:
  // A value. Implicit, like the conversions below, so that a function returns
  // its value or its error as it is: a Result is either, and nothing else.
  constexpr Result(const T& value)  // NOLINT(runtime/explicit)
      : value_(value) {}

  // An error, and T() as the value.
  constexpr Result(const Error& error)  // NOLINT(runtime/explicit)
      : value_(), status_(error) {}

  // A code of a group, as in the implicit constructor of Error.
  template <typename Code, typename = decltype(GetErrorGroup(Code()))>
  constexpr Result(Code code)  // NOLINT(runtime/explicit)
      : value_(), status_(code) {}

  bool IsOk() const { return status_.IsOk(); }

  // The error, or null if IsOk(), as Status::error().
  const Error* error() const { return status_.error(); }

  // Ok, or the error: to compare with a code or to print.
  const Status& status() const { return status_; }

  // The value, or T() if !IsOk(): ask first, or use ValueOr().
  const T& value() const { return value_; }

  // Returns the value, or `fallback` if there is an error.
  T ValueOr(const T& fallback) const { return IsOk() ? value_ : fallback; }

  // Returns `f(value())` if there is no error, or else the error. `f` returns
  // a Result of any type, or a Status.
  template <typename F>
  auto AndThen(F f) const -> decltype(f(internal::DeclVal<const T&>())) {
    if (!IsOk()) {
      return *error();
    }
    return f(value_);
  }

  // Returns a Result of `f(value())` if there is no error, or else the error.
  template <typename F>
  auto Map(F f) const -> Result<decltype(f(internal::DeclVal<const T&>()))> {
    if (!IsOk()) {
      return *error();
    }
    return f(value_);
  }

  // Returns this if there is no error, or else `f(*error())`: `f` gets an Error
  // that is one and returns a value in place of the missing one, another
  // error, or a Result.
  template <typename F>
  Result OrElse(F f) const {
    if (IsOk()) {
      return *this;
    }
    return f(*error());
  }

 private:
  T value_;
  Status status_;
};

}  // namespace utl

#endif  // ERRORGROUP_SRC_UTL_RESULT_H_
