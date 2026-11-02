// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A value or none: std::optional, from C++17, for gnu++11.

#ifndef UTL_SRC_UTL_OPTIONAL_H_
#define UTL_SRC_UTL_OPTIONAL_H_

#include "utl/error.h"
#include "utl/misuse.h"
#include "utl/optional_error.h"
#include "utl/result.h"

namespace utl {

// The type of nullopt, as std::nullopt_t: not default-constructible, so that
// `optional = {}` stays an empty optional rather than a nullopt_t.
struct nullopt_t {
  constexpr explicit nullopt_t(int) {}
};

// No value, as std::nullopt.
constexpr nullopt_t nullopt{0};

// A value of T, or none, as std::optional, with its names, as Google allows
// for an STL-like entity:
//
//   utl::optional<uint8_t> Find(char c) {
//     ...
//     return utl::nullopt;
//   }
//
//   if (const utl::optional<uint8_t> index = Find(c)) { Use(*index); }
//   Find(c).value_or(0);
//   Find(c).ok_or(FontError::kUnknownChar);  // a Result, with your error
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
  constexpr optional(const T& value)  // NOLINT(runtime/explicit)
      : value_(value), has_value_(true) {}

  constexpr bool has_value() const { return has_value_; }
  constexpr explicit operator bool() const { return has_value_; }

  // Unchecked, as in std::optional: T() if empty. With UTL_CHECK_ACCESS at 1
  // an empty one is reported first.
  T& operator*() { return static_cast<void>(Checked()), value_; }
  constexpr const T& operator*() const {
    return static_cast<void>(Checked()), value_;
  }
  T* operator->() { return static_cast<void>(Checked()), &value_; }
  constexpr const T* operator->() const {
    return static_cast<void>(Checked()), &value_;
  }

  // The value, or OptionalError::kEmpty. A copy, as a Result holds a value.
  Result<T> value() const {
    if (!has_value_) {
      return OptionalError::kEmpty;
    }
    return value_;
  }

  constexpr T value_or(const T& fallback) const {
    return has_value_ ? value_ : fallback;
  }

  // The value, or `error`, as Rust's ok_or(): an error of the caller's own
  // group where value() would give OptionalError::kEmpty.
  Result<T> ok_or(const Error& error) const {
    if (!has_value_) {
      return error;
    }
    return value_;
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
  friend bool operator==(const optional& a, const optional& b) {
    return a.has_value_ == b.has_value_ &&
           (!a.has_value_ || a.value_ == b.value_);
  }
  friend bool operator!=(const optional& a, const optional& b) {
    return !(a == b);
  }

 private:
  constexpr bool Checked() const {
    return internal::Check(has_value_, Misuse::kOptionalValue);
  }

  T value_;
  bool has_value_;
};

}  // namespace utl

#endif  // UTL_SRC_UTL_OPTIONAL_H_
