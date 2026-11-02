// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The group an error comes from. A group is one object per program, and its
// address is what makes it unique: no one has to hand out group ids, and a
// component adds a group of its own without changing this library. See
// docu/ErrorHandling.md of MaxMatrix, "Domains: what makes one unique", for
// the alternatives that were weighed.

#ifndef ERRORGROUP_SRC_UTL_ERROR_GROUP_H_
#define ERRORGROUP_SRC_UTL_ERROR_GROUP_H_

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

namespace utl {

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

  // Returns the text of `code`, from flash, or "unknown code".
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
                                                  size_t count, uint8_t code);

  // The same for a table with data() and size(), a utl::array in PROGMEM:
  //
  //   const utl::array<const char*, 2> kMessages PROGMEM = {{...}};
  //   return LookUpMessage(kMessages, code);
  //
  // A template rather than a parameter of utl::array, which this header
  // cannot include: array.h needs Result, Result needs Error, and Error needs
  // this header.
  template <typename Table>
  static const __FlashStringHelper* LookUpMessage(const Table& messages,
                                                  uint8_t code) {
    return LookUpMessage(messages.data(), messages.size(), code);
  }
};

}  // namespace utl

#endif  // ERRORGROUP_SRC_UTL_ERROR_GROUP_H_
