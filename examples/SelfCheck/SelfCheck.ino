// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Checks the library on the board, or in simavr through tools/self_check.sh,
// and prints one line per check and the number of checks that failed.
// SelfCheck.expected is what a correct run prints.

#include <ErrorGroup.h>
#include <avr/sleep.h>

#include "alpha_error.h"
#include "beta_error.h"

using self_check::AlphaError;
using self_check::BetaError;
using utl::Error;
using utl::Result;

static_assert(sizeof(Error) == 3, "an Error is a pointer and a byte");
static_assert(sizeof(Result<uint8_t>) == 4, "a Result is a value and an Error");

namespace {

// Volatile, so that the compiler cannot fold the checks away and the virtual
// calls really go through the vtable.
volatile uint8_t codes[] = {0, 1, 7};

uint8_t failed = 0;

void Check(bool passed, const __FlashStringHelper* name) {
  Serial.print(passed ? F("pass ") : F("FAIL "));
  Serial.println(name);
  if (!passed) {
    ++failed;
  }
}

void PrintError(const Error& error) {
  if (!error.IsOk()) {
    Serial.print(error.group()->Name());
    Serial.print(F(": "));
  }
  Serial.println(error.Message());
}

// Returns codes[index], or BetaError::kFirst past the end.
Result<uint8_t> GetCode(uint8_t index) {
  if (index >= sizeof(codes)) {
    return BetaError::kFirst;
  }
  // Read first: a const reference does not bind to a volatile.
  const uint8_t code = codes[index];
  return code;
}

void CheckError() {
  const Error ok;
  const Error alpha_first(self_check::kAlphaErrorGroup, codes[0]);
  const Error alpha_second(self_check::kAlphaErrorGroup, codes[1]);
  const Error beta_first(self_check::kBetaErrorGroup, codes[0]);
  const Error beta_unknown(self_check::kBetaErrorGroup, codes[2]);

  PrintError(ok);
  PrintError(alpha_first);
  PrintError(alpha_second);
  PrintError(beta_first);
  PrintError(beta_unknown);

  Check(ok.IsOk() && ok.group() == nullptr, F("Error() is ok"));
  Check(!alpha_first.IsOk(), F("an error is not ok"));
  Check(alpha_first == AlphaError::kFirst, F("compares with its enum"));
  Check(AlphaError::kFirst == alpha_first, F("with the enum on the left"));
  Check(alpha_first != AlphaError::kSecond, F("differs in the code"));
  Check(beta_first != AlphaError::kFirst, F("differs in the group"));
  Check(alpha_first != beta_first, F("same code, other group"));

  const Error braced{BetaError::kFirst};
  const Error converted = BetaError::kFirst;
  Check(braced == beta_first && converted == beta_first,
        F("made from its enum"));
  Check(self_check::MakeAlphaSecondElsewhere() == alpha_second,
        F("equal across translation units"));

  Error assigned;
  assigned = beta_first;
  Check(assigned == BetaError::kFirst, F("assignable"));
}

void CheckErrorFluent() {
  const Error alpha_first = AlphaError::kFirst;
  uint8_t calls = 0;

  Check(Error().AndThen([] { return Error(AlphaError::kSecond); }) ==
            AlphaError::kSecond,
        F("Error::AndThen goes on without an error"));
  const Error stopped = alpha_first.AndThen([&calls] {
    ++calls;
    return Error();
  });
  Check(stopped == AlphaError::kFirst && calls == 0,
        F("Error::AndThen stops at an error"));
  Check(Error().AndThen([] { return Result<uint8_t>(5); }).ValueOr(0) == 5,
        F("Error::AndThen into a Result"));

  Check(alpha_first.OrElse([](const Error&) { return Error(); }).IsOk(),
        F("Error::OrElse recovers"));
  Check(alpha_first
                .OrElse([](const Error& error) {
                  return error == AlphaError::kFirst ? Error(BetaError::kFirst)
                                                     : Error();
                })
                .group() == &self_check::kBetaErrorGroup,
        F("Error::OrElse gets the error"));
  calls = 0;
  const Error untouched = Error().OrElse([&calls](const Error&) {
    ++calls;
    return Error(AlphaError::kFirst);
  });
  Check(untouched.IsOk() && calls == 0, F("Error::OrElse leaves no error"));
}

void CheckResult() {
  uint8_t calls = 0;

  Check(GetCode(1).IsOk() && GetCode(1).value() == 1,
        F("Result holds a value"));
  Check(GetCode(9).error() == BetaError::kFirst, F("Result holds an error"));
  Check(GetCode(2).ValueOr(42) == 7 && GetCode(9).ValueOr(42) == 42,
        F("Result::ValueOr"));

  const Result<bool> mapped =
      GetCode(2).Map([](uint8_t code) { return code == 7; });
  Check(mapped.IsOk() && mapped.value(), F("Result::Map"));
  const Result<bool> not_mapped = GetCode(9).Map([&calls](uint8_t) {
    ++calls;
    return true;
  });
  Check(not_mapped.error() == BetaError::kFirst && calls == 0,
        F("Result::Map keeps the error"));

  const Result<uint8_t> chained =
      GetCode(0)
          .AndThen([](uint8_t code) { return GetCode(code + 1); })
          .AndThen([](uint8_t code) { return GetCode(code + 1); });
  Check(chained.ValueOr(0) == 7, F("Result::AndThen chains"));
  calls = 0;
  const Result<uint8_t> broken =
      GetCode(1)
          .AndThen([](uint8_t code) { return GetCode(code + 8); })
          .AndThen([&calls](uint8_t code) {
            ++calls;
            return GetCode(code);
          });
  Check(broken.error() == BetaError::kFirst && calls == 0,
        F("Result::AndThen stops at an error"));
  const Error checked = GetCode(2).AndThen([](uint8_t code) {
    return code == 7 ? Error() : Error(AlphaError::kFirst);
  });
  Check(checked.IsOk(), F("Result::AndThen into an Error"));

  const Result<uint8_t> recovered =
      GetCode(9).OrElse([](const Error&) -> uint8_t { return 3; });
  Check(recovered.IsOk() && recovered.value() == 3, F("Result::OrElse"));
  calls = 0;
  const Result<uint8_t> kept = GetCode(2).OrElse([&calls](const Error&) {
    ++calls;
    return Result<uint8_t>(0);
  });
  Check(kept.value() == 7 && calls == 0, F("Result::OrElse keeps a value"));
}

}  // namespace

void setup() {
  Serial.begin(115200);

  CheckError();
  CheckErrorFluent();
  CheckResult();

  Serial.print(F("failed: "));
  Serial.println(failed);
  Serial.flush();

  // Sleeping with interrupts off ends simavr; on a board it just stops.
  cli();
  sleep_cpu();
}

void loop() {}
