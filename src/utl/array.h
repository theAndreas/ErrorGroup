// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// std::array, which avr-gcc lacks, with at() returning a Result.

#ifndef ERRORGROUP_SRC_UTL_ARRAY_H_
#define ERRORGROUP_SRC_UTL_ARRAY_H_

#include <stddef.h>

#include "utl/index_error.h"
#include "utl/internal/type_traits.h"
#include "utl/result.h"

namespace utl {

// A C array of N elements that knows its size, is copied and compared as a
// whole, and goes through a range-based for:
//
//   utl::array<uint8_t, 3> rows = {{0x18, 0x3c, 0x18}};
//   for (uint8_t row : rows) { ... }
//   rows.at(5).ValueOr(0);  // 0, as 5 lies beyond the end
//
// It follows std::array and its names, as Google allows for an STL-like
// entity, so that code written for std::array compiles with it. The one
// change: at() cannot throw, as an Arduino sketch is built without
// exceptions, and returns a Result with IndexError::kOutOfRange instead.
//
// A struct with a public member, like std::array: so it is an aggregate, is
// initialized like a C array, and can lie in PROGMEM as one.
template <typename T, size_t N>
struct array {
  // A C array of no elements is ill-formed, and nothing here needs one.
  static_assert(N > 0, "utl::array needs at least one element");

  using value_type = T;
  using size_type = size_t;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using iterator = T*;
  using const_iterator = const T*;

  // The one place the C array is meant.
  T elements[N];  // NOLINT(modernize-avoid-c-arrays)

  constexpr size_type size() const { return N; }
  constexpr size_type max_size() const { return N; }
  constexpr bool empty() const { return false; }

  // Unchecked, as in std::array: an index beyond the end is undefined.
  reference operator[](size_type index) { return elements[index]; }
  constexpr const_reference operator[](size_type index) const {
    return elements[index];
  }

  // The element at `index`, or IndexError::kOutOfRange. A copy, not a
  // reference, as a Result holds a value: to write, check the index and use
  // operator[].
  Result<internal::RemoveCv<T>> at(size_type index) const {
    if (index >= N) {
      return IndexError::kOutOfRange;
    }
    return elements[index];
  }

  reference front() { return elements[0]; }
  constexpr const_reference front() const { return elements[0]; }
  reference back() { return elements[N - 1]; }
  constexpr const_reference back() const { return elements[N - 1]; }

  pointer data() { return elements; }
  constexpr const_pointer data() const { return elements; }

  iterator begin() { return elements; }
  constexpr const_iterator begin() const { return elements; }
  constexpr const_iterator cbegin() const { return elements; }
  iterator end() { return elements + N; }
  constexpr const_iterator end() const { return elements + N; }
  constexpr const_iterator cend() const { return elements + N; }

  void fill(const T& value) {
    for (T& element : elements) {
      element = value;
    }
  }
};

template <typename T, size_t N>
bool operator==(const array<T, N>& a, const array<T, N>& b) {
  for (size_t index = 0; index < N; ++index) {
    if (!(a[index] == b[index])) {
      return false;
    }
  }
  return true;
}

template <typename T, size_t N>
bool operator!=(const array<T, N>& a, const array<T, N>& b) {
  return !(a == b);
}

}  // namespace utl

#endif  // ERRORGROUP_SRC_UTL_ARRAY_H_
