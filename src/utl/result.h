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
// cannot make a Result of nothing. The arguments are passed on as they are, so
// that the T& of a Result<T&> reaches `f` as a T&, not a T const&. Defined
// below Result<void>.
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
    static_assert(internal::IsStatusOrResult<decltype(f(
                      internal::DeclVal<T const&>()))>::kValue,
                  "AndThen: f returns a Status or a Result; for a plain value, "
                  "use Map()");
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

  // Returns the value, or else `f(*error())`, as Rust's unwrap_or_else(): a
  // fallback made only when it is needed, and from the error.
  template <typename F>
  T ValueOrElse(F f) const {
    if (IsOk()) {
      return value_;
    }
    return f(*error());
  }

  // Calls `f(value())` if there is no error, and returns this as it is: to
  // look at the value on its way, as Rust's inspect(). `f` gets a T const&,
  // as it looks and does not change.
  template <typename F>
  Result Inspect(F f) const {
    static_assert(
        internal::IsVoid<decltype(f(internal::DeclVal<T const&>()))>::kValue,
        "Inspect: f returns nothing; it only looks, and what it returned "
        "would be dropped unseen");
    if (IsOk()) {
      f(value_);
    }
    return *this;
  }

  // Calls `f(*error())` if there is one, and returns this as it is, as
  // Status::InspectError().
  template <typename F>
  Result InspectError(F f) const {
    static_assert(internal::IsVoid<decltype(f(*error()))>::kValue,
                  "InspectError: f returns nothing; it only looks, and what "
                  "it returned would be dropped unseen");
    if (!IsOk()) {
      f(*error());
    }
    return *this;
  }

  // Returns this if there is no error, or else the error `f(*error())`
  // returns, as Status::MapError(): an Error or a code, never a value.
  template <typename F>
  Result MapError(F f) const {
    static_assert(internal::IsErrorOrCode<decltype(f(*error()))>::kValue,
                  "MapError: f returns an Error or a code; to recover from the "
                  "error, use OrElse()");
    if (IsOk()) {
      return *this;
    }
    Error const mapped = f(*error());
    return mapped;
  }

 private:
  T value_;
  Status status_;
};

// A reference to a T that lies elsewhere, or an Error: a pointer and a
// Status, 5 bytes on the Uno, for a function that finds an object or fails.
// T needs no default constructor:
//
//   Result<Led&> GetLed(uint8_t pin) {
//     if (pin >= kLeds) {
//       return PanelError::kNoSuchLed;
//     }
//     return leds[pin];
//   }
//
//   GetLed(13).Map([](Led& led) { led.On(); });
//
// As Result<T>, with a T& in place of the value. It binds an lvalue only, as
// a temporary would be gone before the first use, and assigning one rebinds
// it. value() of one with an error is undefined, as there is no T to give,
// where Result<T> gives T(); with UTL_CHECK_ACCESS at 1 that is reported
// first. ValueOr() gives a T&, of the object or of the fallback, which is an
// lvalue for that reason.
template <typename T>
class [[nodiscard]] Result<T&> {
 public:
  // A reference to `object`. Implicit, as the conversions of Result<T>.
  // __builtin_addressof, what std::addressof is built on, takes the address
  // even of a T that overloads unary &.
  constexpr Result(T& object)  // NOLINT(runtime/explicit)
      : pointer_(__builtin_addressof(object)) {}

  // No temporary: deleted, and not explicit, so that a Result<int const&>
  // made from a 5 does not bind it through T const&.
  Result(T&&) = delete;  // NOLINT(runtime/explicit)

  // An error, and no reference.
  constexpr Result(Error const& error)  // NOLINT(runtime/explicit)
      : pointer_(nullptr), status_(error) {}

  // A code of a group, as in the implicit constructor of Error.
  template <typename Code, typename = decltype(GetErrorGroup(Code()))>
  constexpr Result(Code code)  // NOLINT(runtime/explicit)
      : pointer_(nullptr), status_(code) {}

  // One of U& that is also one of T&: of int& to int const&, of Derived& to
  // Base&. Only where a U* converts to a T*, so never of long& to int&.
  template <typename U, typename = decltype(internal::ConvertTo<T*>(
                            internal::DeclVal<U*>()))>
  Result(Result<U&> const& other)  // NOLINT(runtime/explicit)
      : pointer_(other.IsOk() ? __builtin_addressof(other.value()) : nullptr),
        status_(other.status()) {}

  bool IsOk() const { return status_.IsOk(); }
  Error const* error() const { return status_.error(); }
  Status const& status() const { return status_; }

  // The object referred to; undefined if !IsOk(): ask first, or use ValueOr().
  // The analyzer sees the null reference that follows an error; it is what
  // the standard leaves undefined, not an oversight.
  T& value() const {
    // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.UndefReturn)
    return static_cast<void>(internal::Check(IsOk(), Misuse::kResultValue)),
           *pointer_;
  }

  // The object referred to, or `fallback` if there is an error.
  T& ValueOr(T& fallback) const { return IsOk() ? *pointer_ : fallback; }
  // No temporary as the fallback: the T& returned would outlive it.
  T& ValueOr(T&& fallback) const = delete;

  // As in Result<T>, with a T& given to `f`.
  template <typename F>
  auto AndThen(F f) const -> decltype(f(internal::DeclVal<T&>())) {
    static_assert(internal::IsStatusOrResult<decltype(f(
                      internal::DeclVal<T&>()))>::kValue,
                  "AndThen: f returns a Status or a Result; for a plain value, "
                  "use Map()");
    if (!IsOk()) {
      return *error();
    }
    return f(*pointer_);
  }

  template <typename F, typename U = decltype(internal::DeclVal<F&>()(
                            internal::DeclVal<T&>()))>
  Result<U> Map(F f) const {
    if (!IsOk()) {
      return *error();
    }
    return internal::MapCall<U>::Call(f, *pointer_);
  }

  template <typename F>
  Result OrElse(F f) const {
    if (IsOk()) {
      return *this;
    }
    return f(*error());
  }

  // The object referred to, or else the one `f(*error())` refers to. `f`
  // returns a T&, of an object that outlives the call; a T made in `f` would
  // be gone before the caller got to it, so that does not compile.
  template <typename F>
  T& ValueOrElse(F f) const {
    static_assert(
        internal::IsLvalueReference<decltype(f(*error()))>::kValue,
        "f returns a T&: the reference returned would outlive a T made in f");
    if (IsOk()) {
      return *pointer_;
    }
    return f(*error());
  }

  // As in Result<T>: `f` gets a T const&, as it looks and does not change.
  template <typename F>
  Result Inspect(F f) const {
    static_assert(
        internal::IsVoid<decltype(f(internal::DeclVal<T const&>()))>::kValue,
        "Inspect: f returns nothing; it only looks, and what it returned "
        "would be dropped unseen");
    if (IsOk()) {
      f(static_cast<T const&>(*pointer_));
    }
    return *this;
  }

  template <typename F>
  Result InspectError(F f) const {
    static_assert(internal::IsVoid<decltype(f(*error()))>::kValue,
                  "InspectError: f returns nothing; it only looks, and what "
                  "it returned would be dropped unseen");
    if (!IsOk()) {
      f(*error());
    }
    return *this;
  }

  template <typename F>
  Result MapError(F f) const {
    static_assert(internal::IsErrorOrCode<decltype(f(*error()))>::kValue,
                  "MapError: f returns an Error or a code; to recover from the "
                  "error, use OrElse()");
    if (IsOk()) {
      return *this;
    }
    Error const mapped = f(*error());
    return mapped;
  }

 private:
  T* pointer_;
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
  static Result<U> Call(F& f, Args&... args) {
    return f(args...);
  }
};

template <>
struct MapCall<void> {
  template <typename F, typename... Args>
  static Result<void> Call(F& f, Args&... args) {
    f(args...);
    return Result<void>::Ok();
  }
};

}  // namespace internal

}  // namespace utl

#endif  // UTL_SRC_UTL_RESULT_H_
