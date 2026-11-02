// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// std::string_view, from C++17, for gnu++11, for text in RAM; text in flash
// is a flash_string_view.

#ifndef UTL_SRC_UTL_STRING_VIEW_H_
#define UTL_SRC_UTL_STRING_VIEW_H_

#include <stddef.h>
#include <string.h>

#include "utl/index_error.h"
#include "utl/result.h"

namespace utl {

template <typename CharT>
class basic_flash_string_view;

// A view of chars in RAM: a pointer and a size, 4 bytes on the Uno. It owns
// nothing; what it views has to outlive it. A typed line, a buffer or a
// string literal:
//
//   const utl::string_view line(buffer, length);
//   if (line.starts_with("text=")) {
//     line.substr(5).Map(SetText);  // a Result: substr() can fail
//   }
//
// It follows std::string_view and its names, as Google allows for an
// STL-like entity. What std::string_view throws on, at() and substr() beyond
// the end, returns a Result with IndexError::kOutOfRange; find() returns npos,
// as "not found" is no error.
//
// A template, as std::basic_string_view, so that npos can be defined in this
// header: C++11 has no inline variables, and the static member of a class
// would be defined once per file that includes it.
template <typename CharT>
class basic_string_view {
 public:
  using value_type = CharT;
  using size_type = size_t;
  using const_reference = const CharT&;
  using const_pointer = const CharT*;
  using const_iterator = const CharT*;
  using iterator = const_iterator;

  static constexpr size_type npos = static_cast<size_type>(-1);

  // Nothing.
  constexpr basic_string_view() : data_(nullptr), size_(0) {}

  // `size` chars from `data` on.
  constexpr basic_string_view(const CharT* data, size_type size)
      : data_(data), size_(size) {}

  // The chars of a string up to its terminating '\0'. Implicit, as in
  // std::string_view. Not constexpr: a constexpr length in C++11 would be a
  // recursion, one call per char at run time, on a stack of 2 KB.
  basic_string_view(const CharT* text)  // NOLINT(runtime/explicit)
      : data_(text), size_(strlen(text)) {}

  constexpr const_pointer data() const { return data_; }
  constexpr size_type size() const { return size_; }
  constexpr size_type length() const { return size_; }
  constexpr bool empty() const { return size_ == 0; }

  constexpr const_iterator begin() const { return data_; }
  constexpr const_iterator end() const { return data_ + size_; }

  // Unchecked, as in std::string_view: undefined beyond the end, or if
  // empty().
  constexpr const_reference operator[](size_type index) const {
    return data_[index];
  }
  constexpr const_reference front() const { return data_[0]; }
  constexpr const_reference back() const { return data_[size_ - 1]; }

  // The char at `index`, or IndexError::kOutOfRange.
  Result<CharT> at(size_type index) const {
    if (index >= size_) {
      return IndexError::kOutOfRange;
    }
    return data_[index];
  }

  // At most `count` chars from `position` on, or IndexError::kOutOfRange if
  // `position` lies beyond the end. A position of size() gives an empty view,
  // as in std::string_view.
  Result<basic_string_view> substr(size_type position,
                                   size_type count = npos) const {
    if (position > size_) {
      return IndexError::kOutOfRange;
    }
    const size_type rest = size_ - position;
    return basic_string_view(data_ + position, count < rest ? count : rest);
  }

  bool starts_with(basic_string_view prefix) const {
    return prefix.size_ <= size_ && Equal(data_, prefix.data_, prefix.size_);
  }

  // The same with a text in flash, F("text="): flash_string_view.h does the
  // work, reading it with pgm_read, as a pointer into flash read as one into
  // RAM gives whatever lies at that address in RAM.
  bool starts_with(basic_flash_string_view<CharT> prefix) const {
    return prefix.IsPrefixOf(*this);
  }

  bool ends_with(basic_string_view suffix) const {
    return suffix.size_ <= size_ &&
           Equal(data_ + (size_ - suffix.size_), suffix.data_, suffix.size_);
  }

  // The position of the first `c` from `position` on, or npos.
  size_type find(CharT c, size_type position = 0) const {
    for (size_type index = position; index < size_; ++index) {
      if (data_[index] == c) {
        return index;
      }
    }
    return npos;
  }

  // The position of the first `text` from `position` on, or npos.
  size_type find(basic_string_view text, size_type position = 0) const {
    if (text.size_ > size_) {
      return npos;
    }
    for (size_type index = position; index <= size_ - text.size_; ++index) {
      if (Equal(data_ + index, text.data_, text.size_)) {
        return index;
      }
    }
    return npos;
  }

  // Hidden friends, with a string literal or a const CharT* on either side,
  // as both convert.
  friend bool operator==(basic_string_view a, basic_string_view b) {
    return a.size_ == b.size_ && Equal(a.data_, b.data_, a.size_);
  }
  friend bool operator!=(basic_string_view a, basic_string_view b) {
    return !(a == b);
  }
  friend bool operator==(basic_string_view a,
                         basic_flash_string_view<CharT> b) {
    return EqualsFlash(a, b);
  }
  friend bool operator==(basic_flash_string_view<CharT> a,
                         basic_string_view b) {
    return EqualsFlash(b, a);
  }
  friend bool operator!=(basic_string_view a,
                         basic_flash_string_view<CharT> b) {
    return !EqualsFlash(a, b);
  }
  friend bool operator!=(basic_flash_string_view<CharT> a,
                         basic_string_view b) {
    return !EqualsFlash(b, a);
  }

 private:
  // A member, unlike the operators above, so that the friendship of
  // basic_flash_string_view reaches it.
  static bool EqualsFlash(basic_string_view a,
                          basic_flash_string_view<CharT> b) {
    return b.Equals(a);
  }

  static bool Equal(const CharT* a, const CharT* b, size_type count) {
    for (size_type index = 0; index < count; ++index) {
      if (a[index] != b[index]) {
        return false;
      }
    }
    return true;
  }

  const CharT* data_;
  size_type size_;
};

template <typename CharT>
constexpr
    typename basic_string_view<CharT>::size_type basic_string_view<CharT>::npos;

using string_view = basic_string_view<char>;

}  // namespace utl

#endif  // UTL_SRC_UTL_STRING_VIEW_H_
