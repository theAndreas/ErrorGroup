// Checks the library on the board, or in simavr through tools/self_check.sh,
// and prints one line per check and the number of checks that failed.
// SelfCheck.expected is what a correct run prints.

#include <avr/sleep.h>
#include <error_group.h>

#include "test_error.h"

using error_group::Error;
using error_group::MakeError;
using self_check::AlphaError;
using self_check::BetaError;

static_assert(sizeof(Error) == 3, "an Error is a pointer and a byte");

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

}  // namespace

void setup() {
  Serial.begin(115200);

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
  Check(alpha_first != AlphaError::kSecond, F("differs in the code"));
  Check(beta_first != AlphaError::kFirst, F("differs in the group"));
  Check(alpha_first != beta_first, F("same code, other group"));
  Check(MakeError(BetaError::kFirst) == beta_first, F("MakeError"));
  Check(self_check::MakeAlphaSecondElsewhere() == alpha_second,
        F("equal across translation units"));

  Error assigned;
  assigned = beta_first;
  Check(assigned == BetaError::kFirst, F("assignable"));

  Serial.print(F("failed: "));
  Serial.println(failed);
  Serial.flush();

  // Sleeping with interrupts off ends simavr; on a board it just stops.
  cli();
  sleep_cpu();
}

void loop() {}
