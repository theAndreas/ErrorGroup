// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A string_view of text in flash, PROGMEM or F("..."), which has no
// counterpart in the standard.

#ifndef UTL_SRC_UTL_FLASH_STRING_VIEW_H_
#define UTL_SRC_UTL_FLASH_STRING_VIEW_H_

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stddef.h>

#include "utl/index_error.h"
#include "utl/result.h"
#include "utl/string_view.h"

namespace utl {

// A view of chars in flash: a pointer into flash and a size, 4 bytes on the
// Uno. On the AVR flash is an address space of its own: a pointer into it
// read as one into RAM gives whatever lies at that address in RAM, so every
// char here is read with pgm_read_byte(). It takes the type F() gives, so
// that a pointer into RAM cannot be passed by mistake:
//
//   const utl::flash_string_view keyword = F("text=");
//   if (line.starts_with(keyword)) { ... }  // line, a string_view in RAM
//   if (line == F("clear")) { ... }
//
// Its names follow string_view, as far as it goes: operator[] and front()
// return a char rather than a reference, as there is nothing in RAM to refer
// to, and at() and substr() return a Result as there. A template, as
// basic_string_view, so that npos can be defined in this header.
template <typename CharT>
class basic_flash_string_view {
  static_assert(sizeof(CharT) == 1, "flash is read a byte at a time");

 public:
  using value_type = CharT;
  using size_type = size_t;

  static constexpr size_type npos = static_cast<size_type>(-1);

  // Walks the chars, reading each from flash.
  class const_iterator {
   public:
    explicit const_iterator(const CharT* position) : position_(position) {}
    CharT operator*() const {
      return static_cast<CharT>(pgm_read_byte(position_));
    }
    const_iterator& operator++() {
      ++position_;
      return *this;
    }
    friend bool operator==(const_iterator a, const_iterator b) {
      return a.position_ == b.position_;
    }
    friend bool operator!=(const_iterator a, const_iterator b) {
      return !(a == b);
    }

   private:
    const CharT* position_;
  };
  using iterator = const_iterator;

  // Nothing.
  constexpr basic_flash_string_view() : data_(nullptr), size_(0) {}

  // The chars of a text in flash up to its '\0'. Implicit, so that F("...")
  // passes where a flash_string_view is taken.
  basic_flash_string_view(
      const __FlashStringHelper* text)  // NOLINT(runtime/explicit)
      : data_(reinterpret_cast<const CharT*>(text)),
        size_(strlen_P(reinterpret_cast<PGM_P>(text))) {}

  // `size` chars of a text in flash, as from a PROGMEM array:
  // flash_string_view(reinterpret_cast<const __FlashStringHelper*>(kText), 4).
  basic_flash_string_view(const __FlashStringHelper* data, size_type size)
      : data_(reinterpret_cast<const CharT*>(data)), size_(size) {}

  // For Serial.print(), which prints up to the '\0', not size() chars.
  const __FlashStringHelper* data() const {
    return reinterpret_cast<const __FlashStringHelper*>(data_);
  }
  constexpr size_type size() const { return size_; }
  constexpr size_type length() const { return size_; }
  constexpr bool empty() const { return size_ == 0; }

  const_iterator begin() const { return const_iterator(data_); }
  const_iterator end() const { return const_iterator(data_ + size_); }

  // Unchecked, as in string_view: undefined beyond the end, or if empty().
  CharT operator[](size_type index) const {
    return static_cast<CharT>(pgm_read_byte(data_ + index));
  }
  CharT front() const { return (*this)[0]; }
  CharT back() const { return (*this)[size_ - 1]; }

  // The char at `index`, or IndexError::kOutOfRange.
  Result<CharT> at(size_type index) const {
    if (index >= size_) {
      return IndexError::kOutOfRange;
    }
    return (*this)[index];
  }

  // At most `count` chars from `position` on, or IndexError::kOutOfRange if
  // `position` lies beyond the end, as string_view::substr().
  Result<basic_flash_string_view> substr(size_type position,
                                         size_type count = npos) const {
    if (position > size_) {
      return IndexError::kOutOfRange;
    }
    const size_type rest = size_ - position;
    return basic_flash_string_view(data_ + position,
                                   count < rest ? count : rest);
  }

  bool starts_with(basic_string_view<CharT> prefix) const {
    return prefix.size() <= size_ &&
           memcmp_P(prefix.data(), data_, prefix.size()) == 0;
  }

  // The position of the first `c` from `position` on, or npos.
  size_type find(CharT c, size_type position = 0) const {
    for (size_type index = position; index < size_; ++index) {
      if ((*this)[index] == c) {
        return index;
      }
    }
    return npos;
  }

  friend bool operator==(basic_flash_string_view a, basic_flash_string_view b) {
    if (a.size_ != b.size_) {
      return false;
    }
    for (size_type index = 0; index < a.size_; ++index) {
      if (a[index] != b[index]) {
        return false;
      }
    }
    return true;
  }
  friend bool operator!=(basic_flash_string_view a, basic_flash_string_view b) {
    return !(a == b);
  }

 private:
  // For basic_string_view, which compares with a text in flash but cannot
  // include this header, as this one includes it.
  friend class basic_string_view<CharT>;

  basic_flash_string_view(const CharT* data, size_type size)
      : data_(data), size_(size) {}

  bool Equals(basic_string_view<CharT> text) const {
    return text.size() == size_ && memcmp_P(text.data(), data_, size_) == 0;
  }

  bool IsPrefixOf(basic_string_view<CharT> text) const {
    return size_ <= text.size() && memcmp_P(text.data(), data_, size_) == 0;
  }

  // In flash.
  const CharT* data_;
  size_type size_;
};

template <typename CharT>
constexpr typename basic_flash_string_view<CharT>::size_type
    basic_flash_string_view<CharT>::npos;

using flash_string_view = basic_flash_string_view<char>;

}  // namespace utl

#endif  // UTL_SRC_UTL_FLASH_STRING_VIEW_H_
