// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// What a function returns that has nothing else to return: ok, or an error.

#ifndef UTL_SRC_UTL_STATUS_H_
#define UTL_SRC_UTL_STATUS_H_

#include <Arduino.h>
#include <stddef.h>

#include "utl/error.h"
#include "utl/internal/type_traits.h"

namespace utl {

template <typename T>
class Result;

namespace internal {

// What the functions passed to the fluent functions may return, checked with
// a static_assert and a message that says what to use instead: the error
// that a template deep in AndThen() or MapError() gives otherwise names
// neither. Each takes the type as the function returns it, by value, by
// reference or const.

// kValue: whether T is a Status or a Result, what AndThen() goes on with.
// The specialization for Status lies below it, as it needs the class.
template <typename T>
struct IsStatusOrResultImpl {
  static constexpr bool kValue = false;
};
template <typename T>
struct IsStatusOrResultImpl<Result<T>> {
  static constexpr bool kValue = true;
};
template <typename T>
struct IsStatusOrResult {
  static constexpr bool kValue = IsStatusOrResultImpl<RemoveCvRef<T>>::kValue;
};

// kValue: whether T is an Error or a code of a group, what MapError() turns
// an error into; never a Status, which could be ok.
template <typename T, typename = void>
struct IsErrorOrCodeImpl {
  static constexpr bool kValue = false;
};
template <typename T>
struct IsErrorOrCodeImpl<T, Void<decltype(GetErrorGroup(T()))>> {
  static constexpr bool kValue = true;
};
template <>
struct IsErrorOrCodeImpl<Error, void> {
  static constexpr bool kValue = true;
};
template <typename T>
struct IsErrorOrCode {
  static constexpr bool kValue = IsErrorOrCodeImpl<RemoveCvRef<T>>::kValue;
};

}  // namespace internal

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
//
// Not [[nodiscard]], unlike Result and Error: a sketch drops the Status of a
// call that draws or sets, SetText("Hi!"), as a matter of course, and with
// the attribute on the type each such call warned and needed a
// static_cast<void>. A library marks the function whose Status must not be
// dropped itself, [[nodiscard]] Status Begin().
class Status {
 public:
  // Ok. Status::Ok() says the same where it is returned.
  constexpr Status() : error_(nullptr) {}
  static constexpr Status Ok() { return Status(); }

  // An error. Implicit, as a function that returns a Status returns its error
  // as it is.
  constexpr Status(Error const& error)  // NOLINT(runtime/explicit)
      : error_(error) {}

  // A code of a group, as in the implicit constructor of Error. Without it,
  // `return FontError::kUnknownChar;` would need two conversions in a row,
  // which C++ does not make implicitly.
  template <typename Code, typename = decltype(GetErrorGroup(Code()))>
  constexpr Status(Code code)  // NOLINT(runtime/explicit)
      : error_(code) {}

  bool IsOk() const { return error_.IsNone(); }

  // The error, or null if IsOk(). A pointer, so that no one gets hold of an
  // Error that is none.
  Error const* error() const { return IsOk() ? nullptr : &error_; }

  // Returns the text of the error, or "ok", from flash.
  __FlashStringHelper const* Message() const;

  // Prints "ok", or the error as Error::PrintTo() does. Returns the number of
  // chars.
  size_t PrintTo(Print& out) const;

  // Returns `f()` if there is no error, or else the error; `f` is not called
  // then. `f` returns a Status or a Result.
  template <typename F>
  auto AndThen(F f) const -> decltype(f()) {
    static_assert(internal::IsStatusOrResult<decltype(f())>::kValue,
                  "AndThen: f returns a Status or a Result; for a plain value, "
                  "use Map()");
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

  // Calls `f(error)` if there is one, and returns this as it is: to look at
  // an error on its way, as Rust's inspect_err(), to print it, say.
  template <typename F>
  Status InspectError(F f) const {
    static_assert(internal::IsVoid<decltype(f(error_))>::kValue,
                  "InspectError: f returns nothing; it only looks, and what "
                  "it returned would be dropped unseen");
    if (!IsOk()) {
      f(error_);
    }
    return *this;
  }

  // Returns this if there is no error, or else the error `f(error)` returns,
  // as Rust's map_err(): to say an error of a lower layer in a group of one's
  // own. `f` returns an Error or a code, never ok, which only OrElse() makes.
  template <typename F>
  Status MapError(F f) const {
    static_assert(internal::IsErrorOrCode<decltype(f(error_))>::kValue,
                  "MapError: f returns an Error or a code; to recover from the "
                  "error, use OrElse()");
    if (IsOk()) {
      return *this;
    }
    Error const mapped = f(error_);
    return mapped;
  }

  // Calls `f(error)` if there is one, and returns nothing: the end of a
  // chain, where the error is dealt with and nothing is left to look at.
  // Only here, on a Status: on a Result<T> it would drop the value unseen,
  // which is what [[nodiscard]] there keeps from happening.
  template <typename F>
  void ConsumeError(F f) const {
    static_assert(internal::IsVoid<decltype(f(error_))>::kValue,
                  "ConsumeError: f returns nothing; a Status or a Result it "
                  "returned would be dropped unseen");
    if (!IsOk()) {
      f(error_);
    }
  }

  // Hidden friends, with the conversions above on either side, so that a
  // Status compares with another, an Error or a code. Two ok ones are equal,
  // as both hold the same null group and code 0.
  friend bool operator==(Status const& a, Status const& b) {
    return a.error_ == b.error_;
  }
  friend bool operator!=(Status const& a, Status const& b) { return !(a == b); }

 private:
  // With a null group if ok.
  Error error_;
};

namespace internal {

template <>
struct IsStatusOrResultImpl<Status> {
  static constexpr bool kValue = true;
};

}  // namespace internal

}  // namespace utl

#endif  // UTL_SRC_UTL_STATUS_H_
