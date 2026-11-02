// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Errors that carry the group they come from, identified by its address.
//
// An Error is a pointer to its ErrorGroup and a code within that group. A
// group is one object per program, and its address is what makes it unique:
// no one has to hand out group ids, and a component adds a group of its own
// without changing this library. See docu/ErrorHandling.md of MaxMatrix,
// "Domains: what makes one unique", for the alternatives that were weighed.

#ifndef ERRORGROUP_SRC_ERROR_GROUP_H_
#define ERRORGROUP_SRC_ERROR_GROUP_H_

#include <Arduino.h>
#include <stdint.h>

namespace error_group {

// The group an error belongs to, with its name and the text of each code.
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
                                                  uint8_t count, uint8_t code);
};

// An error: the group it comes from and a code within that group, 3 bytes on
// the Uno. A default-constructed Error means "no error"; it is the only one
// whose group is null, so no group has to keep a code free for it.
class Error {
 public:
  // No error.
  constexpr Error() : group_(nullptr), code_(0) {}

  // `code` of `group`.
  constexpr Error(const ErrorGroup& group, uint8_t code)
      : group_(&group), code_(code) {}

  bool IsOk() const { return group_ == nullptr; }

  // Null if IsOk().
  const ErrorGroup* group() const { return group_; }
  uint8_t code() const { return code_; }

  // Returns the text of the error, or "ok", from flash.
  const __FlashStringHelper* Message() const;

  friend bool operator==(const Error& a, const Error& b) {
    return a.group_ == b.group_ && a.code_ == b.code_;
  }
  friend bool operator!=(const Error& a, const Error& b) { return !(a == b); }

 private:
  const ErrorGroup* group_;
  uint8_t code_;
};

// Makes an Error from the code enum of a group. The group is found through
// `GetErrorGroup(code)`, which each group declares beside its enum, in the
// same namespace, so that argument-dependent lookup finds it.
template <typename Code>
constexpr Error MakeError(Code code) {
  return Error(GetErrorGroup(code), static_cast<uint8_t>(code));
}

// Compares an error with a code of a group: the group and the code must both
// match, so codes of different groups may overlap. Takes part in overload
// resolution only for an enum that has a GetErrorGroup(): a template that
// takes any type would win against an operator== of another type that the
// error converts to, and then fail inside.
template <typename Code, typename = decltype(GetErrorGroup(Code()))>
bool operator==(const Error& error, Code code) {
  return error == MakeError(code);
}

template <typename Code, typename = decltype(GetErrorGroup(Code()))>
bool operator!=(const Error& error, Code code) {
  return !(error == code);
}

}  // namespace error_group

#endif  // ERRORGROUP_SRC_ERROR_GROUP_H_
