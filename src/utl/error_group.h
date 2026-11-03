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

namespace utl {

// The group an error belongs to: the address of its name, a text in flash
// that no other group shares. 2 bytes, a value that is copied and compared.
//
// A group is its name, an enum class of its codes, and beside the enum, in the
// same namespace, a function that argument-dependent lookup finds:
//
//   // font_error.h; "font" is defined in font_error.cpp
//   extern char const kFontErrorGroup[] PROGMEM;
//   constexpr utl::ErrorGroup GetErrorGroup(FontError) {
//     return utl::ErrorGroup(kFontErrorGroup);
//   }
//
// Define the name exactly once, in one .cpp file, and declare it `extern` in
// the header: a `const` array defined in a header has internal linkage, every
// translation unit would get a copy at an address of its own, and errors made
// in different files would not compare equal. Two groups may share the words
// of their names, never the array.
//
// The texts of the codes are not part of it: a sketch that wants them in
// words turns them on with utl::UseMessages(), see messages.h. A virtual
// Message(), as before, linked every text of every group into any sketch
// that made an error, printed or not, as GCC cannot drop a virtual function
// no one calls; and it put a vtable into RAM, which only a compiler plugin
// moved to flash. A group object, as after that, still took 2 bytes of RAM,
// as avr-gcc keeps const data there; the name lies in flash already.
class ErrorGroup {
 public:
  // The group whose name is `name`, an array in PROGMEM.
  constexpr explicit ErrorGroup(char const* name) : name_(name) {}

  // Returns the name of the group, from flash.
  __FlashStringHelper const* Name() const {
    return reinterpret_cast<__FlashStringHelper const*>(name_);
  }

  friend constexpr bool operator==(ErrorGroup a, ErrorGroup b) {
    return a.name_ == b.name_;
  }
  friend constexpr bool operator!=(ErrorGroup a, ErrorGroup b) {
    return a.name_ != b.name_;
  }

 private:
  friend class Error;

  // No group, which only an Error inside a Status holds, for "ok".
  constexpr explicit ErrorGroup(decltype(nullptr)) : name_(nullptr) {}

  char const* name_;
};

}  // namespace utl

#endif  // UTL_SRC_UTL_ERROR_GROUP_H_
