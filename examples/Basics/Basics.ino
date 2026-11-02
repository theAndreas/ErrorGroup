// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Two groups that the library does not know, errors from both, and how a
// sketch checks, chains and prints them.

#include <ErrorGroup.h>

#include "bus_error.h"
#include "font_error.h"

using bus::BusError;
using font::FontError;
using utl::Error;
using utl::Result;

namespace {

// Fails on a char the font lacks. Takes its input from a pin in loop(), so
// that the compiler cannot fold the error away.
Error SetChar(char c) {
  if (c < ' ') {
    return FontError::kUnknownChar;
  }
  return Error();
}

// Returns the char the other side sent back, or the error of the bus.
Result<char> Receive(bool acknowledged) {
  if (!acknowledged) {
    return BusError::kNack;
  }
  return 'A';
}

void PrintError(const Error& error) {
  if (error.IsOk()) {
    Serial.println(error.Message());
    return;
  }
  Serial.print(error.group()->Name());
  Serial.print(F(": "));
  Serial.println(error.Message());
}

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

void loop() {
  // Receive a char and show it; either step may fail, and the first error
  // ends the chain.
  const Error error =
      Receive(digitalRead(3) == HIGH).AndThen([](char received) {
        return SetChar(digitalRead(2) == LOW ? '\n' : received);
      });
  PrintError(error);

  // Both codes are 0, in different groups: only the group tells them apart.
  if (error == FontError::kUnknownChar) {
    Serial.println(F("pick another char"));
  }
  if (error == BusError::kNack) {
    Serial.println(F("retry"));
  }

  const Error unknown_char{FontError::kUnknownChar};
  if (SetChar('\n') == unknown_char) {
    Serial.println(F("a control char is not in the font"));
  }
  delay(1000);
}
