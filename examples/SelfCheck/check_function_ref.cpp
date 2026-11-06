// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// function_ref, as a parameter above all: the pointer to a function on the
// AVR is a word address, which has to come back as it went in.

#include <Utl.h>
#include <stdint.h>

#include "self_check.h"

namespace self_check {
namespace {

uint8_t Doubled(uint8_t value) { return static_cast<uint8_t>(2 * value); }
uint8_t Zero(uint8_t /*value*/) { return 0; }

// Adds what f gives for each code: a function_ref as a parameter, what it is
// made for.
uint8_t SumOfCodes(utl::function_ref<uint8_t(uint8_t)> f) {
  uint8_t sum = 0;
  for (uint8_t const code : codes) {
    sum = static_cast<uint8_t>(sum + f(code));
  }
  return sum;
}

}  // namespace

void CheckFunctionRef() {
  // The codes are 0, 1 and 7.
  Check(SumOfCodes(Doubled) == 16, F("function_ref calls a function"));

  uint8_t offset = codes[1];
  auto const add = [&offset](uint8_t code) {
    return static_cast<uint8_t>(code + offset);
  };
  Check(SumOfCodes(add) == 11, F("function_ref calls a lambda"));
  offset = codes[2];
  utl::function_ref<uint8_t(uint8_t)> f = add;
  Check(f(0) == 7, F("function_ref refers to the lambda, not a copy"));

  f = Zero;
  utl::function_ref<uint16_t(uint8_t)> const widened = Doubled;
  Check(f(codes[2]) == 0 && widened(codes[2]) == 14,
        F("function_ref rebinds, and to a function of other types"));
  Check(SumOfCodes([](uint8_t code) { return code; }) == 8,
        F("function_ref of a lambda written as the argument"));
}

}  // namespace self_check
