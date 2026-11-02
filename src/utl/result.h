// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A value or the error that kept a function from returning one.

#ifndef ERRORGROUP_SRC_UTL_RESULT_H_
#define ERRORGROUP_SRC_UTL_RESULT_H_

#include "utl/error.h"

namespace utl {
namespace internal {

// std::declval, as avr-gcc comes without a C++ standard library. Declared
// only: it names a value of type T in decltype, where nothing is evaluated.
template <typename T>
T&& DeclVal();

}  // namespace internal

// A value of T, or an error. The two lie side by side, not in a union: T needs
// a default constructor, for the value of a Result with an error, and in
// return the class needs no tag, no placement new and no deviation under a
// safety standard's rules on unions.
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
      : value_(value), error_() {}

  // An error, and T() as the value. An Error() makes a Result without error,
  // whose value is T().
  constexpr Result(const Error& error)  // NOLINT(runtime/explicit)
      : value_(), error_(error) {}

  // A code of a group, as in the implicit constructor of Error.
  template <typename Code, typename = decltype(GetErrorGroup(Code()))>
  constexpr Result(Code code)  // NOLINT(runtime/explicit)
      : value_(), error_(code) {}

  bool IsOk() const { return error_.IsOk(); }

  // Error() if IsOk().
  const Error& error() const { return error_; }

  // The value, or T() if !IsOk(): ask first, or use ValueOr().
  const T& value() const { return value_; }

  // Returns the value, or `fallback` if there is an error.
  T ValueOr(const T& fallback) const { return IsOk() ? value_ : fallback; }

  // Returns `f(value())` if there is no error, or else the error. `f` returns
  // a Result of any type, or an Error.
  template <typename F>
  auto AndThen(F f) const -> decltype(f(internal::DeclVal<const T&>())) {
    if (!IsOk()) {
      return error_;
    }
    return f(value_);
  }

  // Returns a Result of `f(value())` if there is no error, or else the error.
  template <typename F>
  auto Map(F f) const -> Result<decltype(f(internal::DeclVal<const T&>()))> {
    if (!IsOk()) {
      return error_;
    }
    return f(value_);
  }

  // Returns this if there is no error, or else `f(error())`: `f` returns a
  // value in place of the missing one, another error, or a Result.
  template <typename F>
  Result OrElse(F f) const {
    if (IsOk()) {
      return *this;
    }
    return f(error_);
  }

 private:
  T value_;
  Error error_;
};

}  // namespace utl

#endif  // ERRORGROUP_SRC_UTL_RESULT_H_
