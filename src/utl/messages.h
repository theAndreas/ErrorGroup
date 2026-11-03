// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The texts of a group's codes, for a sketch that wants its errors in words.

#ifndef UTL_SRC_UTL_MESSAGES_H_
#define UTL_SRC_UTL_MESSAGES_H_

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "utl/array.h"
#include "utl/error_group.h"

namespace utl {

// The texts of one group, which UseMessages() hands to every Error of that
// group. A group offers a function that makes one and turns it on, beside its
// table:
//
//   // font_error.cpp
//   char const kUnknownCharText[] PROGMEM = "unknown char";
//   constexpr auto kMessages PROGMEM = utl::make_array(kUnknownCharText);
//
//   void UseFontMessages() {
//     static utl::Messages messages{utl::ErrorGroup(kFontErrorGroup),
//                                   kMessages};
//     utl::UseMessages(messages);
//   }
//
// The table is linked only into a sketch that calls UseFontMessages(), as
// nothing else names it: a sketch pays for the texts it asks for, and the
// others for none, which a switch for the whole build could not tell apart.
// The object costs 7 bytes of RAM, in the sketch that calls it. Name the group
// by its array rather than through a function: the static has to be
// initialized when compiled, or it costs a guard.
class Messages {
 public:
  // The table of `group`, in PROGMEM, in the order of its codes. A code is a
  // uint8_t, so a group has at most 255 texts.
  template <size_t N>
  constexpr Messages(ErrorGroup group, array<char const*, N> const& texts)
      : group_(group), texts_(texts.data()), count_(static_cast<uint8_t>(N)) {
    // Not UINT8_MAX: avr-libc defines it in C++ only with __STDC_LIMIT_MACROS.
    static_assert(static_cast<uint8_t>(N) == N,
                  "a code is a uint8_t: a group has at most 255 texts");
  }

  Messages(Messages const&) = delete;
  Messages& operator=(Messages const&) = delete;

  // Returns the text of `code` of `group`, from flash, or null if the group's
  // texts are off or have none for the code; Error then prints the code, which
  // says more than "unknown code" would, and costs no text.
  static __FlashStringHelper const* Find(ErrorGroup group, uint8_t code);

 private:
  friend void UseMessages(Messages& messages);

  ErrorGroup group_;
  char const* const* texts_;
  uint8_t count_;
  // The next group whose texts are on, a list through the objects that
  // UseMessages() was given, so that it takes no memory of its own.
  Messages* next_ = nullptr;
};

// Turns on the texts of `messages`' group: from now on Error::Message() and
// Error::PrintTo() find them. Once is enough; a second call does nothing.
// `messages` has to live as long as the program, as a static one does.
void UseMessages(Messages& messages);

}  // namespace utl

#endif  // UTL_SRC_UTL_MESSAGES_H_
