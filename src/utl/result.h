// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A value or the error that kept a function from returning one.

#ifndef UTL_SRC_UTL_RESULT_H_
#define UTL_SRC_UTL_RESULT_H_

#include "utl/error.h"
#include "utl/internal/type_traits.h"
#include "utl/misuse.h"
#include "utl/status.h"

namespace utl {

template <typename T>
class Result;

namespace internal {

// Calls `f(args...)` and makes a Result<U> of what it returns, for Map(). The
// specialization for void calls `f` and returns ok, as `return f(args...);`
// cannot make a Result of nothing. Defined below Result<void>.
template <typename U>
struct MapCall;

}  // namespace internal

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
class [[nodiscard]] Result {
 public:
  // A value. Implicit, like the conversions below, so that a function returns
  // its value or its error as it is: a Result is either, and nothing else.
  constexpr Result(T const& value)  // NOLINT(runtime/explicit)
      : value_(value) {}

  // An error, and T() as the value.
  constexpr Result(Error const& error)  // NOLINT(runtime/explicit)
      : value_(), status_(error) {}

  // A code of a group, as in the implicit constructor of Error.
  template <typename Code, typename = decltype(GetErrorGroup(Code()))>
  constexpr Result(Code code)  // NOLINT(runtime/explicit)
      : value_(), status_(code) {}

  bool IsOk() const { return status_.IsOk(); }

  // The error, or null if IsOk(), as Status::error().
  Error const* error() const { return status_.error(); }

  // Ok, or the error: to compare with a code or to print.
  Status const& status() const { return status_; }

  // The value, or T() if !IsOk(): ask first, or use ValueOr().
  T const& value() const {
    return static_cast<void>(internal::Check(IsOk(), Misuse::kResultValue)),
           value_;
  }

  // Returns the value, or `fallback` if there is an error.
  T ValueOr(T const& fallback) const { return IsOk() ? value_ : fallback; }

  // Returns `f(value())` if there is no error, or else the error. `f` returns
  // a Result of any type, or a Status.
  template <typename F>
  auto AndThen(F f) const -> decltype(f(internal::DeclVal<T const&>())) {
    if (!IsOk()) {
      return *error();
    }
    return f(value_);
  }

  // Returns a Result of `f(value())` if there is no error, or else the error.
  // A Result<void> if `f` returns nothing: it is called for what it does.
  template <typename F, typename U = decltype(internal::DeclVal<F&>()(
                            internal::DeclVal<T const&>()))>
  Result<U> Map(F f) const {
    if (!IsOk()) {
      return *error();
    }
    return internal::MapCall<U>::Call(f, value_);
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

// The Result of a function that returns no value: ok or an Error, that is, a
// Status, as Rust's Result<(), E>. It is a Status, so that it has all of it,
// IsOk(), error(), AndThen(), OrElse() and the comparisons, and converts to
// one and back. It exists for code that is generic in T, above all Map() with
// a function that returns nothing; a function of a sketch returns a Status.
// Not [[nodiscard]], as a Status is not: it has no value to lose.
template <>
class Result<void> : public Status {
 public:
  // Ok, from an Error or from a code, as a Status.
  using Status::Status;
  constexpr Result() = default;
  static constexpr Result Ok() { return Result(); }

  // A Status, which is the same thing. Implicit, as the conversions of Status.
  constexpr Result(Status const& status)  // NOLINT(runtime/explicit)
      : Status(status) {}

  Status const& status() const { return *this; }

  // Returns a Result of `f()` if there is no error, or else the error: makes
  // a value from "ok", as Rust's map() on a Result<(), E>.
  template <typename F, typename U = decltype(internal::DeclVal<F&>()())>
  Result<U> Map(F f) const {
    if (!IsOk()) {
      return *error();
    }
    return internal::MapCall<U>::Call(f);
  }
};

namespace internal {

template <typename U>
struct MapCall {
  template <typename F, typename... Args>
  static Result<U> Call(F& f, Args const&... args) {
    return f(args...);
  }
};

template <>
struct MapCall<void> {
  template <typename F, typename... Args>
  static Result<void> Call(F& f, Args const&... args) {
    f(args...);
    return Result<void>::Ok();
  }
};

}  // namespace internal

}  // namespace utl

#endif  // UTL_SRC_UTL_RESULT_H_
