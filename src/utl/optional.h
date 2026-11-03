// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A value or none: std::optional, from C++17, for gnu++11.

#ifndef UTL_SRC_UTL_OPTIONAL_H_
#define UTL_SRC_UTL_OPTIONAL_H_

#include <stdint.h>

#include "utl/error.h"
#include "utl/internal/type_traits.h"
#include "utl/misuse.h"
#include "utl/optional_error.h"
#include "utl/result.h"

namespace utl {

template <typename T>
class optional;

namespace internal {

// kValue: whether T is an optional, what optional::AndThen() goes on with;
// see IsStatusOrResult in status.h.
template <typename T>
struct IsOptionalImpl {
  static constexpr bool kValue = false;
};
template <typename T>
struct IsOptionalImpl<optional<T>> {
  static constexpr bool kValue = true;
};
template <typename T>
struct IsOptional {
  static constexpr bool kValue = IsOptionalImpl<RemoveCvRef<T>>::kValue;
};

}  // namespace internal

// The type of nullopt, as std::nullopt_t: not default-constructible, so that
// `optional = {}` stays an empty optional rather than a nullopt_t. The
// parameter only keeps it so; its type does not matter.
struct nullopt_t {
  constexpr explicit nullopt_t(uint8_t) {}
};

// No value, as std::nullopt.
constexpr nullopt_t nullopt{0};

// A value of T, or none, as std::optional, with its names, as Google allows
// for an STL-like entity, except where Result does the same: HasValue(),
// ValueOr(), OkOr(), AndThen(), Map() and OrElse() are named as in Result, so
// that "a value or none" and "a value or an error" read alike:
//
//   utl::optional<uint8_t> Find(char c) {
//     ...
//     return utl::nullopt;
//   }
//
//   if (const utl::optional<uint8_t> index = Find(c)) { Use(*index); }
//   Find(c).ValueOr(0);
//   Find(c).OkOr(FontError::kUnknownChar);  // a Result, with your error
//
// For "none" that is no error. A function that can fail returns a Result.
//
// The value and the flag lie side by side, not in a union, as in Result: T
// needs a default constructor, the value of an empty optional, and in return
// there is no placement new and no union. reset() sets the value to T()
// rather than destroying it.
//
// The one change: value() cannot throw bad_optional_access, as an Arduino
// sketch is built without exceptions, and returns a Result with
// OptionalError::kEmpty instead.
template <typename T>
class optional {
 public:
  using value_type = T;

  // Empty.
  constexpr optional() : value_(), has_value_(false) {}
  constexpr optional(nullopt_t)  // NOLINT(runtime/explicit)
      : value_(), has_value_(false) {}

  // A value. Implicit, as in std::optional, so that a function returns its
  // value as it is.
  constexpr optional(T const& value)  // NOLINT(runtime/explicit)
      : value_(value), has_value_(true) {}

  constexpr bool HasValue() const { return has_value_; }
  constexpr explicit operator bool() const { return has_value_; }

  // Unchecked, as in std::optional: T() if empty. With UTL_CHECK_ACCESS at 1
  // an empty one is reported first.
  T& operator*() { return static_cast<void>(Checked()), value_; }
  constexpr T const& operator*() const {
    return static_cast<void>(Checked()), value_;
  }
  T* operator->() { return static_cast<void>(Checked()), &value_; }
  constexpr T const* operator->() const {
    return static_cast<void>(Checked()), &value_;
  }

  // The value, or OptionalError::kEmpty. A copy, as a Result holds a value.
  Result<T> value() const {
    if (!has_value_) {
      return OptionalError::kEmpty;
    }
    return value_;
  }

  constexpr T ValueOr(T const& fallback) const {
    return has_value_ ? value_ : fallback;
  }

  // The value, or `error`, as Rust's ok_or(): an error of the caller's own
  // group where value() would give OptionalError::kEmpty.
  Result<T> OkOr(Error const& error) const {
    if (!has_value_) {
      return error;
    }
    return value_;
  }

  // The fluent functions, named as in Result; C++23's std::optional calls them
  // and_then(), transform() and or_else(). None of them calls `f` when there
  // is no value to give it. AndThen(): `f(value)` if there is a
  // value, `f` returning an optional, or else an empty optional of that type.
  template <typename F>
  auto AndThen(F f) const -> decltype(f(internal::DeclVal<T const&>())) {
    static_assert(internal::IsOptional<decltype(f(
                      internal::DeclVal<T const&>()))>::kValue,
                  "AndThen: f returns an optional; for a plain value, use "
                  "Map()");
    if (!has_value_) {
      return nullopt;
    }
    return f(value_);
  }

  // Map(): an optional of `f(value)` if there is a value, or else an empty
  // one.
  template <typename F, typename U = decltype(internal::DeclVal<F&>()(
                            internal::DeclVal<T const&>()))>
  optional<U> Map(F f) const {
    if (!has_value_) {
      return nullopt;
    }
    return f(value_);
  }

  // OrElse(): this if there is a value, or else `f()`, an optional<T>.
  template <typename F>
  optional OrElse(F f) const {
    if (has_value_) {
      return *this;
    }
    return f();
  }

  // The value, or else `f()`, as Rust's unwrap_or_else(): a fallback made
  // only when it is needed.
  template <typename F>
  T ValueOrElse(F f) const {
    if (has_value_) {
      return value_;
    }
    return f();
  }

  // Calls `f(value)` if there is one, and returns this as it is: to look at
  // the value on its way, as Rust's inspect(). `f` gets a T const&, as it
  // looks and does not change.
  template <typename F>
  optional Inspect(F f) const {
    static_assert(
        internal::IsVoid<decltype(f(internal::DeclVal<T const&>()))>::kValue,
        "Inspect: f returns nothing; it only looks, and what it returned "
        "would be dropped unseen");
    if (has_value_) {
      f(value_);
    }
    return *this;
  }

  // This if there is a value and `predicate(value)` holds, or else an empty
  // one, as Rust's filter().
  template <typename P>
  optional Filter(P predicate) const {
    if (has_value_ && predicate(value_)) {
      return *this;
    }
    return nullopt;
  }

  void reset() {
    value_ = T();
    has_value_ = false;
  }

  // Makes the value from `args`, as T(args...).
  template <typename... Args>
  T& emplace(Args&&... args) {
    value_ = T(static_cast<Args&&>(args)...);
    has_value_ = true;
    return value_;
  }

  // Hidden friends. Two empty optionals are equal, and a value or nullopt
  // converts on either side, so `index == 2` and `index == utl::nullopt`
  // need no overloads of their own.
  friend bool operator==(optional const& a, optional const& b) {
    return a.has_value_ == b.has_value_ &&
           (!a.has_value_ || a.value_ == b.value_);
  }
  friend bool operator!=(optional const& a, optional const& b) {
    return !(a == b);
  }

 private:
  constexpr bool Checked() const {
    return internal::Check(has_value_, Misuse::kOptionalValue);
  }

  T value_;
  bool has_value_;
};

// A reference to a T that lies elsewhere, or none, as std::optional<T&> from
// C++26: a pointer, 2 bytes on the Uno, null if empty, with no flag beside it.
// T needs no default constructor, as there is no value of its own:
//
//   utl::optional<Led&> FindLed(uint8_t pin);
//
//   if (utl::optional<Led&> const led = FindLed(13)) { led->On(); }
//   FindLed(13).ValueOr(spare).On();
//
// Whether it may be empty is what sets it apart from a reference_wrapper; a
// member that always refers to something is one of those.
//
// It binds an lvalue only: a temporary would be gone before the first use.
// Assigning one rebinds it, as in C++26, and never writes through it; to
// write, use * or ->. Its constness is its own: an optional<T&> const still
// gives a T&.
//
// The changes, beyond the names of optional<T>: * and -> of an empty one are
// undefined, as in the standard, as there is no T to give; with
// UTL_CHECK_ACCESS at 1 that is reported first. And ValueOr() gives a T&, of
// the object referred to or of the fallback, which is an lvalue for that
// reason, where C++26's value_or() gives a copy: a copy would let
// `FindLed(13).ValueOr(spare).On()` switch on a copy of the LED, and copy it
// first.
template <typename T>
class optional<T&> {
 public:
  using value_type = T;

  // Empty.
  constexpr optional() : pointer_(nullptr) {}
  constexpr optional(nullopt_t)  // NOLINT(runtime/explicit)
      : pointer_(nullptr) {}

  // A reference to `object`. Implicit, as in std::optional, so that a
  // function returns the object as it is. __builtin_addressof, what
  // std::addressof is built on, takes the address even of a T that overloads
  // unary &.
  constexpr optional(T& object)  // NOLINT(runtime/explicit)
      : pointer_(__builtin_addressof(object)) {}

  // No temporary, as for reference_wrapper: deleted, and not explicit, so
  // that `optional<int const&> o = 5` does not bind the 5 through T const&.
  optional(T&&) = delete;  // NOLINT(runtime/explicit)

  // One of U& that is also one of T&: of int& to int const&, of Derived& to
  // Base&. Only where a U* converts to a T*, so never of long& to int&.
  template <typename U, typename = decltype(internal::ConvertTo<T*>(
                            internal::DeclVal<U*>()))>
  constexpr optional(optional<U&> const& other)  // NOLINT(runtime/explicit)
      : pointer_(other.HasValue() ? __builtin_addressof(*other) : nullptr) {}

  constexpr bool HasValue() const { return pointer_ != nullptr; }
  constexpr explicit operator bool() const { return pointer_ != nullptr; }

  // Undefined if empty, as in std::optional<T&>. With UTL_CHECK_ACCESS at 1
  // an empty one is reported first. The analyzer sees the null reference that
  // follows; it is what the standard leaves undefined, not an oversight.
  constexpr T& operator*() const {
    // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.UndefReturn)
    return static_cast<void>(Checked()), *pointer_;
  }
  constexpr T* operator->() const {
    return static_cast<void>(Checked()), pointer_;
  }

  // The reference, or OptionalError::kEmpty.
  Result<T&> value() const {
    if (pointer_ == nullptr) {
      return OptionalError::kEmpty;
    }
    return *pointer_;
  }

  // The object referred to, or `fallback` if empty: a reference either way.
  T& ValueOr(T& fallback) const {
    return pointer_ != nullptr ? *pointer_ : fallback;
  }
  // No temporary as the fallback: the T& returned would outlive it.
  T& ValueOr(T&& fallback) const = delete;

  // The reference, or `error`, as Rust's ok_or().
  Result<T&> OkOr(Error const& error) const {
    if (pointer_ == nullptr) {
      return error;
    }
    return *pointer_;
  }

  // As in optional<T>, with a T& given to `f`: AndThen() returns what `f`
  // returns, an optional, Map() an optional of it, OrElse() this or `f()`.
  // None of them calls `f` when there is nothing to give it.
  template <typename F>
  auto AndThen(F f) const -> decltype(f(internal::DeclVal<T&>())) {
    static_assert(
        internal::IsOptional<decltype(f(internal::DeclVal<T&>()))>::kValue,
        "AndThen: f returns an optional; for a plain value, use "
        "Map()");
    if (pointer_ == nullptr) {
      return nullopt;
    }
    return f(*pointer_);
  }

  template <typename F, typename U = decltype(internal::DeclVal<F&>()(
                            internal::DeclVal<T&>()))>
  optional<U> Map(F f) const {
    if (pointer_ == nullptr) {
      return nullopt;
    }
    return f(*pointer_);
  }

  template <typename F>
  optional OrElse(F f) const {
    if (pointer_ != nullptr) {
      return *this;
    }
    return f();
  }

  // The object referred to, or else the one `f()` refers to. `f` returns a
  // T&, of an object that outlives the call, as for ValueOr().
  template <typename F>
  T& ValueOrElse(F f) const {
    static_assert(
        internal::IsLvalueReference<decltype(f())>::kValue,
        "f returns a T&: the reference returned would outlive a T made in f");
    if (pointer_ != nullptr) {
      return *pointer_;
    }
    return f();
  }

  // As in optional<T>: `f` and `predicate` get a T const&, as they look and
  // do not change.
  template <typename F>
  optional Inspect(F f) const {
    static_assert(
        internal::IsVoid<decltype(f(internal::DeclVal<T const&>()))>::kValue,
        "Inspect: f returns nothing; it only looks, and what it returned "
        "would be dropped unseen");
    if (pointer_ != nullptr) {
      f(static_cast<T const&>(*pointer_));
    }
    return *this;
  }

  template <typename P>
  optional Filter(P predicate) const {
    if (pointer_ != nullptr && predicate(static_cast<T const&>(*pointer_))) {
      return *this;
    }
    return nullopt;
  }

  void reset() { pointer_ = nullptr; }

  // Rebinds to `object`, as in C++26.
  T& emplace(T& object) {
    pointer_ = __builtin_addressof(object);
    return object;
  }
  T& emplace(T&& object) = delete;

  // Hidden friends. Two compare by the objects they refer to, as in C++26,
  // not by where these lie; two empty ones are equal. A T compares with the
  // object referred to, and nullopt converts, so `led == utl::nullopt` needs
  // no overload of its own.
  friend bool operator==(optional const& a, optional const& b) {
    return a.pointer_ == nullptr
               ? b.pointer_ == nullptr
               : b.pointer_ != nullptr && *a.pointer_ == *b.pointer_;
  }
  friend bool operator!=(optional const& a, optional const& b) {
    return !(a == b);
  }
  friend bool operator==(optional const& a, T const& b) {
    return a.pointer_ != nullptr && *a.pointer_ == b;
  }
  friend bool operator==(T const& a, optional const& b) { return b == a; }
  friend bool operator!=(optional const& a, T const& b) { return !(a == b); }
  friend bool operator!=(T const& a, optional const& b) { return !(b == a); }

 private:
  constexpr bool Checked() const {
    return internal::Check(pointer_ != nullptr, Misuse::kOptionalValue);
  }

  T* pointer_;
};

}  // namespace utl

#endif  // UTL_SRC_UTL_OPTIONAL_H_
