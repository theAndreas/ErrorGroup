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

// The group an error belongs to, and its name.
//
// A group is an object of this class, an enum class of its codes, and beside
// the enum, in the same namespace, a function that argument-dependent lookup
// finds:
//
//   char const kFontName[] PROGMEM = "font";
//   constexpr utl::ErrorGroup kFontErrorGroup{kFontName};  // in font_error.cpp
//   extern utl::ErrorGroup const kFontErrorGroup;           // in font_error.h
//   inline utl::ErrorGroup const& GetErrorGroup(FontError) {
//     return kFontErrorGroup;
//   }
//
// Define exactly one object for each group, in one .cpp file, and declare it
// `extern` in the header. A `const` object defined in a header has internal
// linkage: every translation unit would get its own copy at its own address,
// and errors from different files would not compare equal.
//
// The texts of the codes are not part of it: a sketch that wants them in
// words turns them on with utl::UseMessages(), see messages.h. A virtual
// Message(), as before, linked every text of every group into any sketch
// that made an error, printed or not, as GCC cannot drop a virtual function
// no one calls; and it put a vtable into RAM, which only a compiler plugin
// moved to flash. A group is now the 2 bytes of its name's address.
class ErrorGroup {
 public:
  // `name`, a text in PROGMEM, such as "font".
  constexpr explicit ErrorGroup(char const* name) : name_(name) {}

  ErrorGroup(ErrorGroup const&) = delete;
  ErrorGroup& operator=(ErrorGroup const&) = delete;

  // Returns the name of the group, from flash.
  __FlashStringHelper const* Name() const {
    return reinterpret_cast<__FlashStringHelper const*>(name_);
  }

 private:
  char const* name_;
};

}  // namespace utl

#endif  // UTL_SRC_UTL_ERROR_GROUP_H_
