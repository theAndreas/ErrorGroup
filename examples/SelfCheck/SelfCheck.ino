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
static_assert(sizeof(utl::optional<uint8_t&>) == 2,
              "an optional of a reference is a pointer");
static_assert(sizeof(Result<uint8_t&>) == 5,
              "a Result of a reference is a pointer and a Status");
static_assert(sizeof(utl::reference_wrapper<uint8_t>) == 2,
              "a reference_wrapper is a pointer");
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
utl::array<uint8_t volatile, 3> codes = {{0, 1, 7}};

uint8_t failed = 0;

void Check(bool passed, __FlashStringHelper const* name) {
  Serial.print(passed ? F("pass ") : F("FAIL "));
  Serial.println(name);
  if (!passed) {
    ++failed;
  }
}

void PrintStatus(Status const& status) {
  status.PrintTo(Serial);
  Serial.println();
}

// Returns codes[index], or BetaError::kFirst past the end.
Result<uint8_t> GetCode(uint8_t index) {
  if (index >= codes.size()) {
    return BetaError::kFirst;
  }
  // Read first: a const reference does not bind to a volatile.
  uint8_t const code = codes[index];
  return code;
}

void CheckError() {
  Error const alpha_first(utl::ErrorGroup(self_check::kAlphaErrorGroup),
                          codes[0]);
  Error const alpha_second(utl::ErrorGroup(self_check::kAlphaErrorGroup),
                           codes[1]);
  Error const beta_first(utl::ErrorGroup(self_check::kBetaErrorGroup),
                         codes[0]);
  Error const beta_unknown(utl::ErrorGroup(self_check::kBetaErrorGroup),
                           codes[2]);

  // Until its texts are on, a group prints its code; then its text.
  PrintStatus(alpha_first);
  self_check::UseAlphaMessages();
  self_check::UseBetaMessages();

  PrintStatus(Status::Ok());
  PrintStatus(alpha_first);
  PrintStatus(alpha_second);
  PrintStatus(beta_first);
  PrintStatus(beta_unknown);
  PrintStatus(Error(utl::ErrorGroup(self_check::kBetaErrorGroup), 10));
  PrintStatus(Error(utl::ErrorGroup(self_check::kBetaErrorGroup), 105));

  // "alpha: alpha first", 18 chars.
  size_t const printed = alpha_first.PrintTo(Serial);
  Serial.println();
  Check(printed == 18, F("PrintTo returns the number of chars"));

  Check(alpha_first.group() == utl::ErrorGroup(self_check::kAlphaErrorGroup) &&
            alpha_first.code() == 0,
        F("an Error has a group and a code"));
  Check(alpha_first == AlphaError::kFirst, F("compares with its enum"));
  Check(AlphaError::kFirst == alpha_first, F("with the enum on the left"));
  Check(alpha_first != AlphaError::kSecond, F("differs in the code"));
  Check(beta_first != AlphaError::kFirst, F("differs in the group"));
  Check(alpha_first != beta_first, F("same code, other group"));

  Error const braced{BetaError::kFirst};
  Error const converted = BetaError::kFirst;
  Check(braced == beta_first && converted == beta_first,
        F("made from its enum"));
  Check(self_check::MakeAlphaSecondElsewhere() == alpha_second,
        F("equal across translation units"));

  Error assigned = alpha_first;
  assigned = beta_first;
  Check(assigned == BetaError::kFirst, F("assignable"));
}

void CheckStatus() {
  Status const ok = Status::Ok();
  Status const alpha_first = AlphaError::kFirst;

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
  Status const alpha_first = AlphaError::kFirst;
  uint8_t calls = 0;

  Check(Status::Ok().AndThen([] { return Status(AlphaError::kSecond); }) ==
            AlphaError::kSecond,
        F("Status::AndThen goes on when ok"));
  Status const stopped = alpha_first.AndThen([&calls] {
    ++calls;
    return Status::Ok();
  });
  Check(stopped == AlphaError::kFirst && calls == 0,
        F("Status::AndThen stops at an error"));
  Check(Status::Ok().AndThen([] { return Result<uint8_t>(5); }).ValueOr(0) == 5,
        F("Status::AndThen into a Result"));

  Check(alpha_first.OrElse([](Error const&) { return Status::Ok(); }).IsOk(),
        F("Status::OrElse recovers"));
  Check(alpha_first.OrElse([](Error const& error) {
    return error == AlphaError::kFirst ? Status(BetaError::kFirst)
                                       : Status::Ok();
  }) == BetaError::kFirst,
        F("Status::OrElse gets the Error"));
  calls = 0;
  Status const untouched = Status::Ok().OrElse([&calls](Error const&) {
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

  Result<bool> const mapped =
      GetCode(2).Map([](uint8_t code) { return code == 7; });
  Check(mapped.IsOk() && mapped.value(), F("Result::Map"));
  Result<bool> const not_mapped = GetCode(9).Map([&calls](uint8_t) {
    ++calls;
    return true;
  });
  Check(not_mapped.status() == BetaError::kFirst && calls == 0,
        F("Result::Map keeps the error"));

  Result<uint8_t> const chained =
      GetCode(0)
          .AndThen([](uint8_t code) { return GetCode(code + 1); })
          .AndThen([](uint8_t code) { return GetCode(code + 1); });
  Check(chained.ValueOr(0) == 7, F("Result::AndThen chains"));
  calls = 0;
  Result<uint8_t> const broken =
      GetCode(1)
          .AndThen([](uint8_t code) { return GetCode(code + 8); })
          .AndThen([&calls](uint8_t code) {
            ++calls;
            return GetCode(code);
          });
  Check(broken.status() == BetaError::kFirst && calls == 0,
        F("Result::AndThen stops at an error"));
  Status const checked = GetCode(2).AndThen([](uint8_t code) {
    return code == 7 ? Status::Ok() : Status(AlphaError::kFirst);
  });
  Check(checked.IsOk(), F("Result::AndThen into a Status"));

  Result<uint8_t> const recovered =
      GetCode(9).OrElse([](Error const&) -> uint8_t { return 3; });
  Check(recovered.IsOk() && recovered.value() == 3, F("Result::OrElse"));
  calls = 0;
  Result<uint8_t> const kept = GetCode(2).OrElse([&calls](Error const&) {
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

  Result<void> const ok = Result<void>::Ok();
  Result<void> const failed_result = AlphaError::kFirst;
  Check(ok.IsOk() && ok.error() == nullptr && Result<void>() == ok,
        F("Result<void>::Ok() is ok"));
  Check(failed_result == AlphaError::kFirst &&
            *failed_result.error() == AlphaError::kFirst,
        F("Result<void> holds its Error"));

  Status const status = failed_result;
  Result<void> const from_status = Status(BetaError::kFirst);
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
  Result<uint8_t> const not_made = failed_result.Map([&calls] {
    ++calls;
    return uint8_t{5};
  });
  Check(not_made.status() == AlphaError::kFirst && calls == 0,
        F("Result<void>::Map keeps the error"));
  Check(failed_result.OrElse([](Error const&) { return Status::Ok(); }).IsOk(),
        F("Result<void> recovers as a Status"));
}

// The element of `rows` at `index`, or BetaError::kFirst past the end: a
// Result of a reference.
Result<uint8_t&> GetRow(utl::array<uint8_t, 3>& rows, uint8_t index) {
  if (index >= rows.size()) {
    return BetaError::kFirst;
  }
  return rows[index];
}

void CheckResultReference() {
  utl::array<uint8_t, 3> rows = {{codes[0], codes[1], codes[2]}};
  uint8_t spare = 9;
  Result<uint8_t&> const found = GetRow(rows, 2);

  Check(found.IsOk() && &found.value() == &rows[2],
        F("Result<T&> holds a reference"));
  Check(GetRow(rows, 3).status() == BetaError::kFirst,
        F("Result<T&> holds an error"));
  Result<void> const set = GetRow(rows, 1).Map([](uint8_t& row) { row = 5; });
  Check(set.IsOk() && rows[1] == 5, F("Result<T&>::Map writes through"));
  Check(&GetRow(rows, 3).ValueOr(spare) == &spare &&
            &found.ValueOr(spare) == &rows[2],
        F("Result<T&>::ValueOr gives a reference"));
  Result<uint8_t&> const chained =
      GetRow(rows, 0).AndThen([&rows](uint8_t& row) {
        return GetRow(rows, static_cast<uint8_t>(row + 2));
      });
  Check(&chained.value() == &rows[2], F("Result<T&>::AndThen chains"));
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
  utl::optional<uint8_t> const none = FindCode(42);
  utl::optional<uint8_t> const found = FindCode(7);

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
  auto const find_again = [](uint8_t index) { return FindCode(index); };
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
  auto const three = [&calls] {
    ++calls;
    return utl::optional<uint8_t>(3);
  };
  Check(none.OrElse(three) == uint8_t{3} && calls == 1 &&
            found.OrElse(three) == uint8_t{2} && calls == 1,
        F("optional::OrElse"));
}

// The element of `rows` equal to `wanted`, or none: an optional of a
// reference, through which the caller writes.
utl::optional<uint8_t&> FindRow(utl::array<uint8_t, 3>& rows, uint8_t wanted) {
  for (uint8_t& row : rows) {
    if (row == wanted) {
      return row;
    }
  }
  return utl::nullopt;
}

void CheckOptionalReference() {
  utl::array<uint8_t, 3> rows = {{codes[0], codes[1], codes[2]}};
  uint8_t spare = 9;
  utl::optional<uint8_t&> const none = FindRow(rows, 42);
  utl::optional<uint8_t&> const found = FindRow(rows, 7);

  Check(!none && none == utl::nullopt && found && &*found == &rows[2],
        F("optional<T&> is empty or refers"));
  *found = 8;
  Check(rows[2] == 8 && found == uint8_t{8}, F("optional<T&> writes through"));
  utl::optional<uint8_t&> rebound = found;
  rebound = spare;
  Check(&*rebound == &spare && rows[2] == 8,
        F("optional<T&> rebinds when assigned"));
  Check(&none.ValueOr(spare) == &spare && &found.ValueOr(spare) == &rows[2],
        F("optional<T&>::ValueOr gives a reference"));
  Check(&found.value().value() == &rows[2] &&
            none.value().status() == utl::OptionalError::kEmpty,
        F("optional<T&>::value returns a Result<T&>"));
  utl::optional<bool> const mapped = found.Map([](uint8_t& row) {
    row = 7;
    return true;
  });
  Check(mapped == true && rows[2] == 7, F("optional<T&>::Map gets the T&"));
}

// The functions after Rust that look on the way, end a chain, or make a
// fallback only when it is needed.
void CheckRustFluent() {
  uint8_t seen = 0;
  auto const look = [&seen](uint8_t const& code) { seen = code; };
  auto const count = [&seen](Error const&) { ++seen; };

  Check(GetCode(2).Inspect(look).ValueOr(0) == 7 && seen == 7,
        F("Result::Inspect"));
  seen = 0;
  Check(
      GetCode(9).InspectError(count).status() == BetaError::kFirst && seen == 1,
      F("Result::InspectError"));
  Check(GetCode(9)
                .MapError([](Error const&) { return AlphaError::kSecond; })
                .status() == AlphaError::kSecond,
        F("Result::MapError"));
  Check(GetCode(9).ValueOrElse([](Error const&) { return uint8_t{3}; }) == 3,
        F("Result::ValueOrElse"));

  // Map() of a function that returns nothing, then ConsumeError(): both ends
  // dealt with, and nothing left that warns when dropped.
  seen = 0;
  GetCode(1).Map([&seen](uint8_t code) { seen = code; }).ConsumeError(count);
  GetCode(9).Map([&seen](uint8_t code) { seen = code; }).ConsumeError(count);
  Check(seen == 2, F("Status::ConsumeError ends a chain"));

  auto const odd = [](uint8_t const& index) { return index % 2 == 1; };
  Check(FindCode(1).Filter(odd) == uint8_t{1} &&
            FindCode(7).Filter(odd) == utl::nullopt,
        F("optional::Filter"));
  Check(FindCode(42).ValueOrElse([] { return uint8_t{5}; }) == 5,
        F("optional::ValueOrElse"));
}

// Counts into a counter another owns: a reference_wrapper as a member, what it
// is made for.
class Counter {
 public:
  explicit Counter(uint8_t& count) : count_(count) {}
  void Increment() { ++count_.get(); }

 private:
  utl::reference_wrapper<uint8_t> count_;
};

void CheckReferenceWrapper() {
  uint8_t first = codes[1];
  uint8_t second = codes[2];
  utl::reference_wrapper<uint8_t> wrapper = first;

  Check(&wrapper.get() == &first, F("reference_wrapper refers to its object"));
  wrapper.get() = 2;
  uint8_t& converted = wrapper;
  Check(first == 2 && &converted == &first,
        F("reference_wrapper writes through get()"));
  wrapper = second;
  Check(&wrapper.get() == &second && first == 2,
        F("reference_wrapper rebinds when assigned"));

  Counter const counter(first);
  Counter copy = counter;
  copy.Increment();
  copy = Counter(second);
  copy.Increment();
  Check(first == 3 && second == 8,
        F("reference_wrapper as a member copies and assigns"));

  utl::reference_wrapper<uint8_t const> const read_only = utl::ref(first);
  Check(&read_only.get() == &first && &utl::cref(second).get() == &second,
        F("ref, cref, and to one of const"));

  utl::array<utl::reference_wrapper<uint8_t>, 2> const both = {{first, second}};
  for (uint8_t& value : both) {
    value = 0;
  }
  Check(first == 0 && second == 0, F("reference_wrapper in an array"));
}

void CheckCharconv() {
  utl::array<char, 8> buffer = {};
  Check(utl::to_chars(buffer, uint8_t{codes[2]}).ValueOr("") == "7" &&
            utl::to_chars(buffer, int16_t{-32768}).ValueOr("") == "-32768" &&
            utl::to_chars(buffer, uint8_t{255}, 16).ValueOr("") == "ff",
        F("to_chars"));
  Check(utl::to_chars(buffer, int32_t{-12345678}).status() ==
            utl::CharconvError::kValueTooLarge,
        F("to_chars beyond the buffer"));

  utl::string_view line = "42,-7";
  Check(utl::from_chars<uint8_t>(line).ValueOr(0) == 42 && line == ",-7",
        F("from_chars takes what it reads"));
  line.remove_prefix(1);
  Check(utl::from_chars<int8_t>(line).ValueOr(0) == -7 && line.empty(),
        F("from_chars of a negative"));
  utl::string_view large = "256";
  Result<uint8_t> const out_of_range = utl::from_chars<uint8_t>(large);
  PrintStatus(out_of_range.status());
  Check(out_of_range.status() == utl::CharconvError::kOutOfRange &&
            large == "256",
        F("from_chars out of range"));
}

void CheckStringView() {
  char buffer[] = "text=Hi";
  utl::string_view const line = buffer;

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

  utl::flash_string_view const keyword = F("text=");
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
  for (char const c : keyword) {
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
  Result<uint8_t> const failed_result = GetCode(9);
  static_cast<void>(failed_result.value());
  Check(
      MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kResultValue,
      F("value() of a Result with an error is reported"));
  before = MisuseCount();
  utl::optional<uint8_t> const none;
  static_cast<void>(*none);
  Check(MisuseCount() == before + 1 &&
            LastMisuse() == utl::Misuse::kOptionalValue,
        F("* of an empty optional is reported"));
  utl::array<uint8_t, 2> pair = {{1, 2}};
  before = MisuseCount();
  static_cast<void>(pair[2]);
  Check(MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kIndex,
        F("[] beyond the end is reported"));
  span<uint8_t> const empty;
  before = MisuseCount();
  static_cast<void>(empty.front());
  Check(MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kEmpty,
        F("front() of an empty span is reported"));
  utl::optional<uint8_t&> const no_row;
  before = MisuseCount();
  static_cast<void>(*no_row);
  Check(MisuseCount() == before + 1 &&
            LastMisuse() == utl::Misuse::kOptionalValue,
        F("* of an empty optional<T&> is reported"));
  Result<uint8_t&> const no_reference = BetaError::kFirst;
  before = MisuseCount();
  static_cast<void>(no_reference.value());
  Check(
      MisuseCount() == before + 1 && LastMisuse() == utl::Misuse::kResultValue,
      F("value() of a Result<T&> with an error is reported"));

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
  for (uint8_t const row : rows) {
    sum += row;
  }
  Check(sum == 8, F("array in a range-based for"));
  Check(rows.at(2).ValueOr(0) == 7, F("array::at"));
  Result<uint8_t> const beyond = rows.at(3);
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
uint16_t Sum(span<uint8_t const> values) {
  uint16_t sum = 0;
  for (uint8_t const value : values) {
    sum += value;
  }
  return sum;
}

void CheckSpan() {
  utl::array<uint8_t, 4> rows = {{codes[0], codes[1], codes[2], 3}};
  utl::array<uint8_t, 4> const& const_rows = rows;
  // The C array is what this checks a span of.
  uint8_t c_array[] = {codes[2], codes[2]};  // NOLINT(modernize-avoid-c-arrays)
  span<uint8_t> const all = rows;

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
  // The library's groups: index in words, optional as a code, so that the
  // output shows both.
  utl::UseIndexMessages();
  utl::UseCharconvMessages();

  CheckError();
  CheckStatus();
  CheckStatusFluent();
  CheckResult();
  CheckResultVoid();
  CheckResultReference();
  CheckArray();
  CheckSpan();
  CheckOptional();
  CheckOptionalReference();
  CheckRustFluent();
  CheckReferenceWrapper();
  CheckStringView();
  CheckCharconv();
  CheckMisuse();

  Serial.print(F("failed: "));
  Serial.println(failed);
  Serial.flush();

  // Sleeping with interrupts off ends simavr; on a board it just stops.
  cli();
  sleep_cpu();
}

void loop() {}
