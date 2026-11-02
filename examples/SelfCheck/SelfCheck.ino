// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Checks the library on the board, or in simavr through tools/self_check.sh,
// and prints one line per check and the number of checks that failed.
// SelfCheck.expected is what a correct run prints.

#include <Utl.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stddef.h>
#include <stdint.h>

#include "alpha_error.h"
#include "beta_error.h"
#if UTL_CHECK_ACCESS
#include "misuse_record.h"
#endif

using self_check::AlphaError;
using self_check::BetaError;
using utl::Error;
using utl::IndexError;
using utl::Result;
using utl::span;
using utl::Status;

static_assert(sizeof(Error) == 3, "an Error is a pointer and a byte");
static_assert(sizeof(Status) == 3,
              "a Status is an Error, with a null group if ok");
static_assert(sizeof(Result<uint8_t>) == 4, "a Result is a value and a Status");
static_assert(sizeof(Result<void>) == 3, "a Result<void> is a Status");
static_assert(sizeof(utl::array<uint8_t, 3>) == 3, "an array is its elements");
static_assert(sizeof(span<uint8_t>) == 4, "a span is a pointer and a size");
static_assert(sizeof(utl::optional<uint8_t>) == 2,
              "an optional is a value and a flag");
static_assert(sizeof(utl::string_view) == 4 &&
                  sizeof(utl::flash_string_view) == 4,
              "a string_view is a pointer and a size");
constexpr utl::array<uint8_t, 3> kConstantRows = {{1, 2, 3}};
static_assert(kConstantRows[1] == 2 && kConstantRows.size() == 3,
              "an array is constexpr");
constexpr auto kMadeRows = utl::make_array(uint8_t{1}, uint8_t{2}, uint8_t{3});
static_assert(kMadeRows.size() == 3 && kMadeRows[2] == 3 &&
                  sizeof(kMadeRows) == 3,
              "make_array takes type and size from its arguments");

namespace {

// Volatile, so that the compiler cannot fold the checks away and the virtual
// calls really go through the vtable.
utl::array<volatile uint8_t, 3> codes = {{0, 1, 7}};

uint8_t failed = 0;

void Check(bool passed, const __FlashStringHelper* name) {
  Serial.print(passed ? F("pass ") : F("FAIL "));
  Serial.println(name);
  if (!passed) {
    ++failed;
  }
}

void PrintStatus(const Status& status) {
  status.PrintTo(Serial);
  Serial.println();
}

// Returns codes[index], or BetaError::kFirst past the end.
Result<uint8_t> GetCode(uint8_t index) {
  if (index >= codes.size()) {
    return BetaError::kFirst;
  }
  // Read first: a const reference does not bind to a volatile.
  const uint8_t code = codes[index];
  return code;
}

void CheckError() {
  const Error alpha_first(self_check::kAlphaErrorGroup, codes[0]);
  const Error alpha_second(self_check::kAlphaErrorGroup, codes[1]);
  const Error beta_first(self_check::kBetaErrorGroup, codes[0]);
  const Error beta_unknown(self_check::kBetaErrorGroup, codes[2]);

  PrintStatus(Status::Ok());
  PrintStatus(alpha_first);
  PrintStatus(alpha_second);
  PrintStatus(beta_first);
  PrintStatus(beta_unknown);
  PrintStatus(Error(self_check::kBetaErrorGroup, 10));
  PrintStatus(Error(self_check::kBetaErrorGroup, 105));

  // "alpha: alpha first", 18 chars, or "alpha: 0", 8, without texts.
  const size_t printed = alpha_first.PrintTo(Serial);
  Serial.println();
  Check(printed == (UTL_ERROR_TEXTS ? 18 : 8),
        F("PrintTo returns the number of chars"));

  Check(&alpha_first.group() == &self_check::kAlphaErrorGroup &&
            alpha_first.code() == 0,
        F("an Error has a group and a code"));
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

  Error assigned = alpha_first;
  assigned = beta_first;
  Check(assigned == BetaError::kFirst, F("assignable"));
}

void CheckStatus() {
  const Status ok = Status::Ok();
  const Status alpha_first = AlphaError::kFirst;

  Check(ok.IsOk() && ok.error() == nullptr && Status() == ok,
        F("Status::Ok() is ok and has no Error"));
  Check(!alpha_first.IsOk() && alpha_first.error() != nullptr &&
            *alpha_first.error() == AlphaError::kFirst,
        F("a Status holds its Error"));
  Check(alpha_first == AlphaError::kFirst && alpha_first != BetaError::kFirst &&
            alpha_first != ok,
        F("Status compares with a code"));
  Check(alpha_first == Error(AlphaError::kFirst) &&
            Error(AlphaError::kFirst) == alpha_first,
        F("Status compares with an Error"));
}

void CheckStatusFluent() {
  const Status alpha_first = AlphaError::kFirst;
  uint8_t calls = 0;

  Check(Status::Ok().AndThen([] { return Status(AlphaError::kSecond); }) ==
            AlphaError::kSecond,
        F("Status::AndThen goes on when ok"));
  const Status stopped = alpha_first.AndThen([&calls] {
    ++calls;
    return Status::Ok();
  });
  Check(stopped == AlphaError::kFirst && calls == 0,
        F("Status::AndThen stops at an error"));
  Check(Status::Ok().AndThen([] { return Result<uint8_t>(5); }).ValueOr(0) == 5,
        F("Status::AndThen into a Result"));

  Check(alpha_first.OrElse([](const Error&) { return Status::Ok(); }).IsOk(),
        F("Status::OrElse recovers"));
  Check(alpha_first.OrElse([](const Error& error) {
    return error == AlphaError::kFirst ? Status(BetaError::kFirst)
                                       : Status::Ok();
  }) == BetaError::kFirst,
        F("Status::OrElse gets the Error"));
  calls = 0;
  const Status untouched = Status::Ok().OrElse([&calls](const Error&) {
    ++calls;
    return Status(AlphaError::kFirst);
  });
  Check(untouched.IsOk() && calls == 0, F("Status::OrElse leaves ok alone"));
}

void CheckResult() {
  uint8_t calls = 0;

  Check(GetCode(1).IsOk() && GetCode(1).value() == 1,
        F("Result holds a value"));
  Check(GetCode(9).status() == BetaError::kFirst &&
            *GetCode(9).error() == BetaError::kFirst,
        F("Result holds an error"));
  Check(GetCode(2).ValueOr(42) == 7 && GetCode(9).ValueOr(42) == 42,
        F("Result::ValueOr"));

  const Result<bool> mapped =
      GetCode(2).Map([](uint8_t code) { return code == 7; });
  Check(mapped.IsOk() && mapped.value(), F("Result::Map"));
  const Result<bool> not_mapped = GetCode(9).Map([&calls](uint8_t) {
    ++calls;
    return true;
  });
  Check(not_mapped.status() == BetaError::kFirst && calls == 0,
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
  Check(broken.status() == BetaError::kFirst && calls == 0,
        F("Result::AndThen stops at an error"));
  const Status checked = GetCode(2).AndThen([](uint8_t code) {
    return code == 7 ? Status::Ok() : Status(AlphaError::kFirst);
  });
  Check(checked.IsOk(), F("Result::AndThen into a Status"));

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

// Stores the code at `index` in `stored`, or fails past the end: a function
// that returns no value, as a Result<void>.
Result<void> StoreCode(uint8_t index, uint8_t& stored) {
  return GetCode(index).Map([&stored](uint8_t code) { stored = code; });
}

void CheckResultVoid() {
  uint8_t stored = 0;
  uint8_t calls = 0;

  const Result<void> ok = Result<void>::Ok();
  const Result<void> failed_result = AlphaError::kFirst;
  Check(ok.IsOk() && ok.error() == nullptr && Result<void>() == ok,
        F("Result<void>::Ok() is ok"));
  Check(failed_result == AlphaError::kFirst &&
            *failed_result.error() == AlphaError::kFirst,
        F("Result<void> holds its Error"));

  const Status status = failed_result;
  const Result<void> from_status = Status(BetaError::kFirst);
  Check(status == AlphaError::kFirst && from_status == BetaError::kFirst &&
            failed_result.status() == AlphaError::kFirst,
        F("Result<void> is a Status, and back"));

  Check(StoreCode(2, stored).IsOk() && stored == 7,
        F("Result::Map with a function that returns nothing"));
  stored = 0;
  Check(StoreCode(9, stored) == BetaError::kFirst && stored == 0,
        F("Result::Map into Result<void> keeps the error"));

  Check(Result<void>::Ok().Map([] { return uint8_t{5}; }).ValueOr(0) == 5,
        F("Result<void>::Map makes a value"));
  const Result<uint8_t> not_made = failed_result.Map([&calls] {
    ++calls;
    return uint8_t{5};
  });
  Check(not_made.status() == AlphaError::kFirst && calls == 0,
        F("Result<void>::Map keeps the error"));
  Check(failed_result.OrElse([](const Error&) { return Status::Ok(); }).IsOk(),
        F("Result<void> recovers as a Status"));
}

// Returns the index of the first code equal to `wanted`, or none.
utl::optional<uint8_t> FindCode(uint8_t wanted) {
  for (size_t index = 0; index < codes.size(); ++index) {
    if (codes[index] == wanted) {
      return static_cast<uint8_t>(index);  // codes has 3 elements
    }
  }
  return utl::nullopt;
}

void CheckOptional() {
  const utl::optional<uint8_t> none = FindCode(42);
  const utl::optional<uint8_t> found = FindCode(7);

  Check(!none.HasValue() && !none && none == utl::nullopt &&
            utl::optional<uint8_t>() == none,
        F("optional is empty"));
  Check(found.HasValue() && static_cast<bool>(found) && *found == 2 &&
            found == uint8_t{2} && found != none,
        F("optional holds a value"));

  PrintStatus(none.value().status());
  Check(found.value().ValueOr(0) == 2 &&
            none.value().status() == utl::OptionalError::kEmpty,
        F("optional::value returns a Result"));
  Check(found.ValueOr(9) == 2 && none.ValueOr(9) == 9, F("optional::ValueOr"));
  Check(none.OkOr(AlphaError::kSecond).status() == AlphaError::kSecond &&
            found.OkOr(AlphaError::kSecond).ValueOr(0) == 2,
        F("optional::OkOr"));

  utl::optional<uint8_t> changed = found;
  changed.reset();
  Check(!changed.HasValue(), F("optional::reset"));
  changed.emplace(5);
  Check(changed.HasValue() && *changed == 5 && changed != found,
        F("optional::emplace"));
  changed = utl::nullopt;
  Check(changed == none, F("optional takes nullopt"));
  changed.emplace(6);
  changed = {};
  Check(!changed.HasValue(), F("optional = {} is empty"));

  uint8_t calls = 0;
  const auto find_again = [](uint8_t index) { return FindCode(index); };
  Check(FindCode(1).AndThen(find_again) == uint8_t{1} &&
            found.AndThen(find_again) == utl::nullopt,
        F("optional::AndThen"));
  Check(found.Map([](uint8_t index) { return index * 10; }) == 20 &&
            none.Map([&calls](uint8_t) {
              ++calls;
              return 1;
            }) == utl::nullopt &&
            calls == 0,
        F("optional::Map"));
  const auto three = [&calls] {
    ++calls;
    return utl::optional<uint8_t>(3);
  };
  Check(none.OrElse(three) == uint8_t{3} && calls == 1 &&
            found.OrElse(three) == uint8_t{2} && calls == 1,
        F("optional::OrElse"));
}

void CheckStringView() {
  char buffer[] = "text=Hi";
  const utl::string_view line = buffer;

  Check(line.size() == 7 && line == "text=Hi" && line != "text=Ho",
        F("string_view of a string"));
  Check(line.starts_with("text=") && !line.starts_with("texts") &&
            line.ends_with("Hi") && !line.ends_with("text=Hi!"),
        F("string_view::starts_with, ends_with"));
  Check(line.find('=') == 4 && line.find("Hi") == 5 &&
            line.find('x', 4) == utl::string_view::npos &&
            line.find("Ho") == utl::string_view::npos,
        F("string_view::find"));
  Check(line.substr(5).ValueOr("") == "Hi" && line.substr(7).value().empty() &&
            line.substr(8).status() == IndexError::kOutOfRange &&
            line.substr(1, 3).ValueOr("") == "ext",
        F("string_view::substr"));
  Check(line.at(0).ValueOr(0) == 't' &&
            line.at(7).status() == IndexError::kOutOfRange,
        F("string_view::at"));

  const utl::flash_string_view keyword = F("text=");
  Check(keyword.size() == 5 && keyword[4] == '=' && keyword.front() == 't' &&
            keyword.back() == '=',
        F("flash_string_view reads flash"));
  Check(line.starts_with(keyword) && !line.starts_with(F("clear")) &&
            keyword.starts_with("tex") && !keyword.starts_with("text=Hi"),
        F("starts_with between RAM and flash"));
  Check(line.substr(5).value() == F("Hi") &&
            F("Hi") == line.substr(5).value() && line != F("text=") &&
            F("text=") != line,
        F("== between RAM and flash"));
  Check(keyword.substr(0, 4).value() == F("text") &&
            keyword.substr(6).status() == IndexError::kOutOfRange,
        F("flash_string_view::substr"));
  Check(keyword.find('=') == 4 &&
            keyword.find('z') == utl::flash_string_view::npos &&
            keyword.at(5).status() == IndexError::kOutOfRange,
        F("flash_string_view::find, at"));
  uint8_t ts = 0;
  for (const char c : keyword) {
    if (c == 't') {
      ++ts;
    }
  }
  Check(ts == 2, F("flash_string_view in a range-based for"));

  utl::string_view trimmed = line;
  trimmed.remove_prefix(5);
  Check(trimmed == "Hi", F("string_view::remove_prefix"));
  trimmed = line;
  trimmed.remove_suffix(3);
  Check(trimmed == "text", F("string_view::remove_suffix"));
  Check(utl::string_view("abc") < "abd" && utl::string_view("ab") < "abc" &&
            utl::string_view("b") > "abc" && line.compare("text=Hi") == 0 &&
            utl::string_view("ab") <= "ab" && utl::string_view("b") >= "a",
        F("string_view::compare and <"));
  Check(utl::string_view("\xe4") > "z", F("string_view compares unsigned"));
  Check(keyword.ends_with("t=") && !keyword.ends_with("text=!") &&
            line.ends_with(F("Hi")) && !line.ends_with(F("Ho")),
        F("ends_with between RAM and flash"));
  Check(keyword.find("xt") == 2 &&
            keyword.find("zz") == utl::flash_string_view::npos &&
            keyword.find("text=!") == utl::flash_string_view::npos,
        F("flash_string_view::find of a text"));
}

// With UTL_CHECK_ACCESS at 1, checks that each misuse is reported once, and a
// right access not at all. The reads beyond the end and of an empty span are
// undefined, but harmless on the AVR, where they read a byte of RAM.
void CheckMisuse() {
#if UTL_CHECK_ACCESS
  using self_check::LastMisuse;
  using self_check::MisuseCount;
  uint8_t before = MisuseCount();
  const Result<uint8_t> failed_result = GetCode(9);
  static_cast<void>(failed_result.value());
  Check(
      MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kResultValue,
      F("value() of a Result with an error is reported"));
  before = MisuseCount();
  const utl::optional<uint8_t> none;
  static_cast<void>(*none);
  Check(MisuseCount() == before + 1 &&
            LastMisuse() == utl::Misuse::kOptionalValue,
        F("* of an empty optional is reported"));
  utl::array<uint8_t, 2> pair = {{1, 2}};
  before = MisuseCount();
  static_cast<void>(pair[2]);
  Check(MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kIndex,
        F("[] beyond the end is reported"));
  const span<uint8_t> empty;
  before = MisuseCount();
  static_cast<void>(empty.front());
  Check(MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kEmpty,
        F("front() of an empty span is reported"));

  utl::string_view shortened = "ab";
  before = MisuseCount();
  shortened.remove_prefix(3);
  Check(MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kRemove &&
            shortened.empty(),
        F("remove_prefix() beyond the end is reported, and empties"));

  before = MisuseCount();
  static_cast<void>(pair[1]);
  static_cast<void>(GetCode(1).value());
  static_cast<void>(*FindCode(7));
  Check(MisuseCount() == before, F("a right access is not reported"));

  utl::PrintMisuse(Serial, utl::Misuse::kIndex);
  Serial.println();
  utl::PrintMisuse(Serial, utl::Misuse::kRemove);
  Serial.println();
#endif
}

void CheckArray() {
  utl::array<uint8_t, 3> rows = {{codes[0], codes[1], codes[2]}};

  Check(rows.size() == 3 && !rows.empty(), F("array::size"));
  Check(rows[2] == 7 && rows.front() == 0 && rows.back() == 7,
        F("array::operator[], front, back"));
  uint16_t sum = 0;
  for (const uint8_t row : rows) {
    sum += row;
  }
  Check(sum == 8, F("array in a range-based for"));
  Check(rows.at(2).ValueOr(0) == 7, F("array::at"));
  const Result<uint8_t> beyond = rows.at(3);
  PrintStatus(beyond.status());
  Check(beyond.status() == IndexError::kOutOfRange,
        F("array::at beyond the end"));
  Check(rows.data() == &rows.front(), F("array::data"));

  utl::array<uint8_t, 3> copy = rows;
  Check(copy == rows, F("array copies and compares"));
  copy.fill(5);
  Check(copy != rows && copy[0] == 5 && copy[2] == 5, F("array::fill"));
}

// Takes a C array, a utl::array and a span alike, through a span of const.
uint16_t Sum(span<const uint8_t> values) {
  uint16_t sum = 0;
  for (const uint8_t value : values) {
    sum += value;
  }
  return sum;
}

void CheckSpan() {
  utl::array<uint8_t, 4> rows = {{codes[0], codes[1], codes[2], 3}};
  const utl::array<uint8_t, 4>& const_rows = rows;
  // The C array is what this checks a span of.
  uint8_t c_array[] = {codes[2], codes[2]};  // NOLINT(modernize-avoid-c-arrays)
  const span<uint8_t> all = rows;

  Check(span<uint8_t>().empty() && span<uint8_t>().data() == nullptr,
        F("span() is empty"));
  Check(all.size() == 4 && all.data() == rows.data() && all.size_bytes() == 4,
        F("span of an array"));
  Check(Sum(rows) == 11 && Sum(const_rows) == 11 && Sum(c_array) == 14 &&
            Sum(all) == 11,
        F("span of const from array, const array, C array, span"));
  all[0] = 9;
  Check(rows[0] == 9 && all.front() == 9 && all.back() == 3,
        F("span writes through"));
  all[0] = 0;

  Check(all.at(3).ValueOr(0) == 3 &&
            all.at(4).status() == IndexError::kOutOfRange,
        F("span::at"));
  Check(Sum(all.first(2).value()) == 1 && Sum(all.last(1).value()) == 3,
        F("span::first, last"));
  Check(Sum(all.subspan(1, 2).value()) == 8 && all.subspan(4).value().empty(),
        F("span::subspan"));
  Check(all.first(5).status() == IndexError::kOutOfRange &&
            all.last(5).status() == IndexError::kOutOfRange &&
            all.subspan(5).status() == IndexError::kOutOfRange &&
            all.subspan(2, 3).status() == IndexError::kOutOfRange,
        F("span beyond the end"));
  Check(all.subspan(1, static_cast<size_t>(-1)).status() ==
            IndexError::kOutOfRange,
        F("span::subspan does not overflow"));
  Check(all.subspan(1)
                .AndThen([](span<uint8_t> rest) { return rest.at(1); })
                .ValueOr(0) == 7,
        F("span with the fluent API"));
}

}  // namespace

void setup() {
  Serial.begin(115200);

  CheckError();
  CheckStatus();
  CheckStatusFluent();
  CheckResult();
  CheckResultVoid();
  CheckArray();
  CheckSpan();
  CheckOptional();
  CheckStringView();
  CheckMisuse();

  Serial.print(F("failed: "));
  Serial.println(failed);
  Serial.flush();

  // Sleeping with interrupts off ends simavr; on a board it just stops.
  cli();
  sleep_cpu();
}

void loop() {}
