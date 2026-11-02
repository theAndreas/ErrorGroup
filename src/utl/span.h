// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// std::span, from C++20, for gnu++11, with what can go out of range returning
// a Result.

#ifndef UTL_SRC_UTL_SPAN_H_
#define UTL_SRC_UTL_SPAN_H_

#include <stddef.h>

#include "utl/array.h"
#include "utl/index_error.h"
#include "utl/internal/type_traits.h"
#include "utl/misuse.h"
#include "utl/result.h"

namespace utl {

// A view of elements that lie one after the other: a pointer and a size, 4
// bytes on the Uno. It owns nothing; whatever it views has to outlive it.
// A function that takes a span takes a C array, a utl::array or a pointer and
// a size alike:
//
//   uint8_t Sum(utl::span<const uint8_t> rows);
//   Sum(rows);                           // a utl::array<uint8_t, 8>
//   Sum(utl::span<const uint8_t>(buffer, length));
//
// It follows std::span and its names, as Google allows for an STL-like
// entity, with a dynamic extent only. What std::span leaves undefined beyond
// the end returns a Result with IndexError::kOutOfRange instead: at(),
// first(), last() and subspan(). operator[] stays unchecked, as in std::span,
// unless UTL_CHECK_ACCESS is 1.
template <typename T>
class span {
 public:
  using element_type = T;
  using value_type = internal::RemoveCv<T>;
  using size_type = size_t;
  using reference = T&;
  using pointer = T*;
  using iterator = T*;

  // Nothing.
  constexpr span() : data_(nullptr), size_(0) {}

  // `size` elements from `data` on.
  constexpr span(T* data, size_type size) : data_(data), size_(size) {}

  // The conversions below are implicit, as in std::span: a span is a view of
  // what it is made from, and a function that takes one should take a C array
  // or a utl::array as it is. Each takes part only if an array of the source
  // may be seen as an array of T, so a span of Derived never becomes a span
  // of Base.

  // A whole C array, which is what it takes.
  template <typename U, size_t N, typename = internal::ArrayConvertible<U, T>>
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  constexpr span(U (&elements)[N])  // NOLINT(runtime/explicit)
      : data_(elements), size_(N) {}

  // A whole utl::array.
  template <typename U, size_t N, typename = internal::ArrayConvertible<U, T>>
  constexpr span(array<U, N>& elements)  // NOLINT(runtime/explicit)
      : data_(elements.data()), size_(N) {}

  // A whole utl::array that is const, for a span of const.
  template <typename U, size_t N,
            typename = internal::ArrayConvertible<const U, T>>
  constexpr span(const array<U, N>& elements)  // NOLINT(runtime/explicit)
      : data_(elements.data()), size_(N) {}

  // Another span, as span<uint8_t> to span<const uint8_t>.
  template <typename U, typename = internal::ArrayConvertible<U, T>>
  constexpr span(const span<U>& other)  // NOLINT(runtime/explicit)
      : data_(other.data()), size_(other.size()) {}

  constexpr pointer data() const { return data_; }
  constexpr size_type size() const { return size_; }
  constexpr size_type size_bytes() const { return size_ * sizeof(T); }
  constexpr bool empty() const { return size_ == 0; }

  // Unchecked, as in std::span: an index beyond the end is undefined. With
  // UTL_CHECK_ACCESS at 1 it is reported first.
  constexpr reference operator[](size_type index) const {
    return static_cast<void>(internal::Check(index < size_, Misuse::kIndex)),
           data_[index];
  }

  // The element at `index`, or IndexError::kOutOfRange. A copy, as a Result
  // holds a value: to write, check the index and use operator[].
  Result<value_type> at(size_type index) const {
    if (index >= size_) {
      return IndexError::kOutOfRange;
    }
    return data_[index];
  }

  // Unchecked, as in std::span: undefined if empty(). With UTL_CHECK_ACCESS
  // at 1 that is reported first.
  constexpr reference front() const {
    return static_cast<void>(internal::Check(size_ != 0, Misuse::kEmpty)),
           data_[0];
  }
  constexpr reference back() const {
    return static_cast<void>(internal::Check(size_ != 0, Misuse::kEmpty)),
           data_[size_ - 1];
  }

  constexpr iterator begin() const { return data_; }
  constexpr iterator end() const { return data_ + size_; }

  // The first `count` elements, or IndexError::kOutOfRange if there are
  // fewer.
  Result<span> first(size_type count) const {
    if (count > size_) {
      return IndexError::kOutOfRange;
    }
    return span(data_, count);
  }

  // The last `count` elements, or IndexError::kOutOfRange if there are fewer.
  Result<span> last(size_type count) const {
    if (count > size_) {
      return IndexError::kOutOfRange;
    }
    return span(data_ + (size_ - count), count);
  }

  // The elements from `offset` to the end, or IndexError::kOutOfRange if
  // `offset` lies beyond it. An offset of size() gives an empty span, as in
  // std::span.
  Result<span> subspan(size_type offset) const {
    if (offset > size_) {
      return IndexError::kOutOfRange;
    }
    return span(data_ + offset, size_ - offset);
  }

  // `count` elements from `offset` on, or IndexError::kOutOfRange if they do
  // not all lie within this span. Written so that offset + count cannot
  // overflow.
  Result<span> subspan(size_type offset, size_type count) const {
    if (offset > size_ || count > size_ - offset) {
      return IndexError::kOutOfRange;
    }
    return span(data_ + offset, count);
  }

 private:
  T* data_;
  size_type size_;
};

}  // namespace utl

#endif  // UTL_SRC_UTL_SPAN_H_
