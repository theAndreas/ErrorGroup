// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The fluent functions that Status, Result<T> and Result<T&> share, once:
// what they do with an error in ErrorFluent, what they do with a value in
// ValueFluent. Each class derives from what it has and keeps what differs
// itself: its storage, value(), ValueOr() and ValueOrElse(), and on a Status
// AndThen() and ConsumeError(). The bases hold nothing, so a Status stays 3
// bytes and a Result its value and a Status.

#ifndef UTL_SRC_UTL_INTERNAL_FLUENT_H_
#define UTL_SRC_UTL_INTERNAL_FLUENT_H_

#include "utl/error.h"
#include "utl/internal/type_traits.h"

namespace utl {

class Status;
template <typename T>
class Result;

namespace internal {

// Calls `f(args...)` and makes a Result<U> of what it returns, for Map(). The
// specialization for void calls `f` and returns ok, as `return f(args...);`
// cannot make a Result of nothing. The arguments are passed on as they are, so
// that the T& of a Result<T&> reaches `f` as a T&, not a T const&. Defined in
// result.h, below Result<void>.
template <typename U>
struct MapCall;

// What the functions passed to the fluent functions may return, checked with
// a static_assert and a message that says what to use instead: the error
// that a template deep in AndThen() or MapError() gives otherwise names
// neither. Each takes the type as the function returns it, by value, by
// reference or const.

// kValue: whether T is a Status or a Result, what AndThen() goes on with.
template <typename T>
struct IsStatusOrResultImpl {
  static constexpr bool kValue = false;
};
template <>
struct IsStatusOrResultImpl<Status> {
  static constexpr bool kValue = true;
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

// What Status, Result<T> and Result<T&> do with an error, each returning a
// Self, the class that derives from it. It asks Self only for IsOk() and
// error(), which each has.
template <typename Self>
class ErrorFluent {
 public:
  // Returns this if there is no error, or else `f(*error())`: `f` gets an
  // Error that is one, and returns another, a value in place of the missing
  // one, or what recovers, Status::Ok() say.
  template <typename F>
  Self OrElse(F f) const {
    if (AsSelf().IsOk()) {
      return AsSelf();
    }
    return f(*AsSelf().error());
  }

  // Calls `f(*error())` if there is one, and returns this as it is: to look at
  // an error on its way, as Rust's inspect_err(), to print it, say.
  template <typename F>
  Self InspectError(F f) const {
    static_assert(IsVoid<decltype(f(DeclVal<Error const&>()))>::kValue,
                  "InspectError: f returns nothing; it only looks, and what "
                  "it returned would be dropped unseen");
    if (!AsSelf().IsOk()) {
      f(*AsSelf().error());
    }
    return AsSelf();
  }

  // Returns this if there is no error, or else the error `f(*error())`
  // returns, as Rust's map_err(): to say an error of a lower layer in a group
  // of one's own. `f` returns an Error or a code, never ok, which only
  // OrElse() makes.
  template <typename F>
  Self MapError(F f) const {
    static_assert(IsErrorOrCode<decltype(f(DeclVal<Error const&>()))>::kValue,
                  "MapError: f returns an Error or a code; to recover from the "
                  "error, use OrElse()");
    if (AsSelf().IsOk()) {
      return AsSelf();
    }
    Error const mapped = f(*AsSelf().error());
    return mapped;
  }

 private:
  // Only Self makes one, and ValueFluent on its way to Self, so that no class
  // derives from ErrorFluent<Other> by mistake.
  friend Self;
  template <typename, typename>
  friend class ValueFluent;
  constexpr ErrorFluent() = default;

  Self const& AsSelf() const { return static_cast<Self const&>(*this); }
};

// What Result<T> and Result<T&> do with a value, and through ErrorFluent,
// which it derives from, with an error: one base for a Result, not two.
// `Value` is what value() gives and what `f` gets, a T const& or a T&. It
// takes the value through UncheckedValue(), which Self keeps private and
// opens to it as a friend, and only when there is one: value() would check
// again what IsOk() has just said, which with UTL_CHECK_ACCESS at 1 cost
// SelfCheck 360 bytes. A function passed returns by value; none of them
// calls it when it has nothing to give.
template <typename Self, typename Value>
class ValueFluent : public ErrorFluent<Self> {
 public:
  // Returns `f(value())` if there is no error, or else the error. `f` returns
  // a Result of any type, or a Status.
  template <typename F>
  auto AndThen(F f) const -> decltype(f(DeclVal<Value>())) {
    static_assert(IsStatusOrResult<decltype(f(DeclVal<Value>()))>::kValue,
                  "AndThen: f returns a Status or a Result; for a plain value, "
                  "use Map()");
    if (!AsSelf().IsOk()) {
      return *AsSelf().error();
    }
    return f(AsSelf().UncheckedValue());
  }

  // Returns a Result of `f(value())` if there is no error, or else the error.
  // A Result<void> if `f` returns nothing, as it is called for what it does;
  // a Result<U&> if it returns a U&.
  template <typename F, typename U = decltype(DeclVal<F&>()(DeclVal<Value>()))>
  Result<U> Map(F f) const {
    if (!AsSelf().IsOk()) {
      return *AsSelf().error();
    }
    return MapCall<U>::Call(f, AsSelf().UncheckedValue());
  }

  // Calls `f(value())` if there is no error, and returns this as it is: to
  // look at the value on its way, as Rust's inspect(). `f` gets a T const&,
  // as it looks and does not change.
  template <typename F>
  Self Inspect(F f) const {
    static_assert(IsVoid<decltype(f(DeclVal<Looked>()))>::kValue,
                  "Inspect: f returns nothing; it only looks, and what it "
                  "returned would be dropped unseen");
    if (AsSelf().IsOk()) {
      f(static_cast<Looked>(AsSelf().UncheckedValue()));
    }
    return AsSelf();
  }

 private:
  // Only Self makes one, as for ErrorFluent.
  friend Self;
  constexpr ValueFluent() = default;

  using Looked = RemoveReference<Value> const&;

  Self const& AsSelf() const { return static_cast<Self const&>(*this); }
};

}  // namespace internal
}  // namespace utl

#endif  // UTL_SRC_UTL_INTERNAL_FLUENT_H_
