// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Two groups that the library does not know, errors from both, and how a
// sketch checks and prints them.

#include <error_group.h>

#include "bus_error.h"
#include "font_error.h"

using bus::BusError;
using error_group::Error;
using error_group::MakeError;
using font::FontError;

namespace {

// Fails on a char the font lacks. Takes its input from a pin in loop(), so
// that the compiler cannot fold the error away.
Error SetChar(char c) {
  if (c < ' ') {
    return MakeError(FontError::kUnknownChar);
  }
  return Error();
}

Error Send(bool acknowledged) {
  if (!acknowledged) {
    return MakeError(BusError::kNack);
  }
  return Error();
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
  const Error char_error = SetChar(digitalRead(2) == LOW ? '\n' : 'A');
  const Error bus_error = Send(digitalRead(3) == HIGH);

  PrintError(char_error);
  PrintError(bus_error);

  // Both codes are 0, in different groups: only the group tells them apart.
  if (bus_error == FontError::kUnknownChar) {
    Serial.println(F("never: a bus error is no font error"));
  }
  if (bus_error == BusError::kNack) {
    Serial.println(F("retry"));
  }
  delay(1000);
}
