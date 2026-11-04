// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Integers to text and back: std::to_chars() and std::from_chars(), from
// C++17, for gnu++11, with a Result in place of std::errc. The steps lie in
// internal/charconv.h.
//
// The base is a uint8_t, where the standard takes an int: it lies in 2..36,
// one that is not gives CharconvError::kInvalidArgument, and on the AVR an
// int takes two registers where the base needs one.

#ifndef UTL_SRC_UTL_CHARCONV_H_
#define UTL_SRC_UTL_CHARCONV_H_

#include <stddef.h>
#include <stdint.h>

#include "utl/charconv_error.h"
#include "utl/internal/charconv.h"
#include "utl/result.h"
#include "utl/span.h"
#include "utl/string_view.h"

namespace utl {

// Writes `value` in `base` into [first, last), as std::to_chars(), and returns
// the end of what it wrote; no '\0'. A minus for a negative value, no plus,
// and digits beyond 9 in lower case. CharconvError::kValueTooLarge if the text
// does not fit, and kInvalidArgument for a base not 2..36, which the standard
// leaves undefined; what lies in [first, last) then is unspecified, as there.
// Integers only: a correct to_chars() of a double would not fit on an Uno,
// and avr-libc has dtostrf().
//
//   char buffer[4];
//   utl::to_chars(buffer, buffer + 4, uint8_t{42})  // buffer + 2, "42"
template <typename T>
Result<char*> to_chars(char* first, char const* last, T value,
                       uint8_t base = internal::kDecimal) {
  static_assert(internal::IsCharconvInteger<T>::kValue,
                "to_chars: T is an integer other than bool; floating point is "
                "left out, see dtostrf() of avr-libc");
  if (!internal::IsBase(base)) {
    return CharconvError::kInvalidArgument;
  }
  bool const negative = internal::SignOf<T>::IsNegative(value);
  char* digits = first;
  if (negative) {
    if (digits == last) {
      return CharconvError::kValueTooLarge;
    }
    *digits++ = '-';
  }
  return internal::WriteDigits(digits, last,
                               internal::Magnitude(value, negative), base);
}

// Writes `value` into `buffer`, as above, and returns the text written: a view
// of the front of `buffer`, to print or to compare.
//
//   utl::array<char, 6> buffer;
//   utl::to_chars(buffer, -273)  // "-273"
template <typename T>
Result<string_view> to_chars(span<char> buffer, T value,
                             uint8_t base = internal::kDecimal) {
  char* const first = buffer.data();
  Result<char*> const end = to_chars(first, first + buffer.size(), value, base);
  if (!end.IsOk()) {
    return *end.error();
  }
  return string_view(first, static_cast<size_t>(end.value() - first));
}

// Reads an integer in `base` from the front of [first, last) into `value`, as
// std::from_chars(), and returns where the number ends: at the first char that
// is not one of its digits, which may be `last`. Only what the standard takes:
// a minus for a signed T, no plus, no space, no "0x"; digits beyond 9 in
// either case. CharconvError::kInvalidArgument if no number begins at `first`
// or for a base not 2..36, kOutOfRange if it does not fit into T; `value` is
// left as it is then.
//
// The one change: on an error there is no pointer, where the standard gives
// one past the digits of a number out of range. A Result holds a value or an
// error, not both; to go on behind such a number, skip its digits.
template <typename T>
Result<char const*> from_chars(char const* first, char const* last, T& value,
                               uint8_t base = internal::kDecimal) {
  static_assert(internal::IsCharconvInteger<T>::kValue,
                "from_chars: T is an integer other than bool; floating point "
                "is left out");
  if (!internal::IsBase(base)) {
    return CharconvError::kInvalidArgument;
  }
  char const* digits = first;
  bool const negative = internal::SignOf<T>::ReadMinus(digits, last);
  internal::UnsignedOf<T> magnitude = 0;
  Result<char const*> const end = internal::ReadDigits(
      digits, last, base, internal::MagnitudeLimit<T>(negative), magnitude);
  if (end.IsOk()) {
    value = internal::FromMagnitude<T>(magnitude, negative);
  }
  return end;
}

// Reads an integer from the front of `text`, as above, and returns it; `text`
// then begins after it. On an error `text` is left as it is.
//
//   utl::string_view line = "42,7";
//   utl::from_chars<uint8_t>(line)  // 42; line is ",7"
template <typename T>
Result<T> from_chars(string_view& text, uint8_t base = internal::kDecimal) {
  T value{};
  char const* const first = text.data();
  Result<char const*> const end =
      from_chars(first, first + text.size(), value, base);
  if (!end.IsOk()) {
    return *end.error();
  }
  text.remove_prefix(static_cast<size_t>(end.value() - first));
  return value;
}

}  // namespace utl

#endif  // UTL_SRC_UTL_CHARCONV_H_
