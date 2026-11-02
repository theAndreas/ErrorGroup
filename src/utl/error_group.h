// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The group an error comes from. A group is one object per program, and its
// address is what makes it unique: no one has to hand out group ids, and a
// component adds a group of its own without changing this library. See
// docu/ErrorHandling.md of MaxMatrix, "Domains: what makes one unique", for
// the alternatives that were weighed.

#ifndef UTL_SRC_UTL_ERROR_GROUP_H_
#define UTL_SRC_UTL_ERROR_GROUP_H_

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "utl/config.h"

namespace utl {

// Declared ahead, against Google's advice, because this header cannot include
// array.h: array.h needs Result, Result needs Error, and Error needs this
// header. LookUpMessage() below only names it; the group's .cpp includes it.
template <typename T, size_t N>
struct array;

// The group an error belongs to, with its name and the text of each code.
//
// A group is a class derived from this one, an enum class of its codes, and
// beside the enum, in the same namespace, a function that argument-dependent
// lookup finds:
//
//   const utl::ErrorGroup& GetErrorGroup(FontError);
//
// Define exactly one object of each derived class, in one .cpp file, and
// declare it `extern` in the header. A `const` object defined in a header has
// internal linkage: every translation unit would get its own copy at its own
// address, and errors from different files would not compare equal.
//
// On the Uno a class with virtual functions keeps its vtable in RAM, as
// avr-gcc reads it with LD from the data space; the texts stay in flash.
class ErrorGroup {
 public:
  ErrorGroup(const ErrorGroup&) = delete;
  ErrorGroup& operator=(const ErrorGroup&) = delete;

  // Returns the name of the group, from flash.
  virtual const __FlashStringHelper* Name() const = 0;

  // Returns the text of `code`, from flash, or "unknown code"; an empty text,
  // never null, if UTL_ERROR_TEXTS is 0.
  virtual const __FlashStringHelper* Message(uint8_t code) const = 0;

 protected:
  constexpr ErrorGroup() = default;

  // Not virtual, and protected so that no one deletes a group through this
  // base: a virtual destructor would pull `operator delete` into every sketch,
  // and a group is never created on the heap.
  ~ErrorGroup() = default;

  // Returns `messages[code]`, read from a table in PROGMEM, or "unknown code"
  // if `code` lies beyond the table.
  static const __FlashStringHelper* LookUpMessage(const char* const* messages,
                                                  uint8_t count, uint8_t code);

  // The same for the table of a group, a utl::array in PROGMEM:
  //
  //   const utl::array<const char*, 2> kMessages PROGMEM = {...};
  //   return LookUpMessage(kMessages, code);
  //
  // The count is a uint8_t, as a code is: a size_t costs 4 bytes of flash
  // more on the Uno. N is known when compiled, so the static_assert makes
  // sure that the count is not cut short.
  //
  // With UTL_ERROR_TEXTS at 0 it returns an empty text and does not touch
  // the table, so that the table and its texts drop out of the build.
  template <size_t N>
  static const __FlashStringHelper* LookUpMessage(
      const array<const char*, N>& messages, uint8_t code) {
    // Not UINT8_MAX: avr-libc defines it in C++ only with __STDC_LIMIT_MACROS.
    static_assert(static_cast<uint8_t>(N) == N,
                  "a code is a uint8_t: a group has at most 255 texts");
#if UTL_ERROR_TEXTS
    return LookUpMessage(messages.data(), static_cast<uint8_t>(N), code);
#else
    static_cast<void>(messages);
    static_cast<void>(code);
    return NoMessage();
#endif
  }

 private:
  // An empty text, in flash, shared by every group, for UTL_ERROR_TEXTS at
  // 0. Not F("") in the template above, which would put one into flash for
  // every size of table.
  static const __FlashStringHelper* NoMessage();
};

}  // namespace utl

#endif  // UTL_SRC_UTL_ERROR_GROUP_H_
