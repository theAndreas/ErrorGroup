// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The report of a misuse, with UTL_CHECK_ACCESS at 1: the default report,
// which prints on Serial, and PrintMisuse(). Without the switch only
// PrintMisuse() is tested, as nothing is reported.

#include <Arduino.h>
#include <gtest/gtest.h>
#include <utl/array.h>
#include <utl/config.h>
#include <utl/misuse.h>
#include <utl/optional.h>
#include <utl/result.h>
#include <utl/span.h>
#include <utl/string_view.h>

#include <string>

#include "test/beta_error.h"
#include "test/string_print.h"

namespace utl_test {
namespace {

using utl::Misuse;

std::string Printed(Misuse misuse) {
  StringPrint out;
  const size_t count = utl::PrintMisuse(out, misuse);
  EXPECT_EQ(count, out.text.size());
  return out.text;
}

TEST(MisuseTest, PrintsEachKind) {
  EXPECT_EQ(Printed(Misuse::kResultValue),
            "utl: value() of a Result with an error");
  EXPECT_EQ(Printed(Misuse::kOptionalValue),
            "utl: * or -> of an empty optional");
  EXPECT_EQ(Printed(Misuse::kIndex), "utl: [] beyond the end");
  EXPECT_EQ(Printed(Misuse::kEmpty), "utl: front() or back() of an empty view");
  EXPECT_EQ(Printed(Misuse::kRemove),
            "utl: remove_prefix() or remove_suffix() beyond the end");
  EXPECT_EQ(Printed(static_cast<Misuse>(200)), "utl: ");
}

#if UTL_CHECK_ACCESS
// What the default report printed for one misuse.
template <typename F>
std::string Reported(F misuse) {
  Serial.output.clear();
  misuse();
  return Serial.output;
}

TEST(MisuseTest, ReportsEachMisuseOnSerial) {
  // The tests form each wrong access but never read it: static_cast<void> of a
  // reference reads nothing, so the sanitizers have nothing to stop. The views
  // point at memory that is there all the same, the array of three below.
  int storage[] = {1, 2, 3};
  const utl::span<int> two(storage, 2);
  const utl::span<int> none(storage + 1, 0);
  const char text[] = "ab";
  const utl::string_view one(text, 1);

  EXPECT_EQ(Reported([] {
              const utl::Result<int> failed = BetaError::kFirst;
              static_cast<void>(failed.value());
            }),
            "utl: value() of a Result with an error\r\n");
  EXPECT_EQ(Reported([] {
              utl::optional<int> empty;
              static_cast<void>(*empty);
              const utl::optional<int>& same = empty;
              static_cast<void>(*same);
            }),
            "utl: * or -> of an empty optional\r\n"
            "utl: * or -> of an empty optional\r\n");
  EXPECT_EQ(Reported([&] {
              static_cast<void>(two[2]);
              static_cast<void>(one[1]);
            }),
            "utl: [] beyond the end\r\n"
            "utl: [] beyond the end\r\n");
  EXPECT_EQ(Reported([&] {
              static_cast<void>(none.front());
              static_cast<void>(none.back());
            }),
            "utl: front() or back() of an empty view\r\n"
            "utl: front() or back() of an empty view\r\n");
  EXPECT_EQ(Reported([] {
              utl::string_view shortened = "ab";
              shortened.remove_prefix(3);
              EXPECT_TRUE(shortened.empty());
              shortened = "ab";
              shortened.remove_suffix(3);
              EXPECT_TRUE(shortened.empty());
            }),
            "utl: remove_prefix() or remove_suffix() beyond the end\r\n"
            "utl: remove_prefix() or remove_suffix() beyond the end\r\n");
}

TEST(MisuseTest, ReportsAnArrayIndexBeyondTheEnd) {
  EXPECT_EQ(Reported([] {
              volatile size_t beyond = 2;
              utl::array<int, 2> pair = {{1, 2}};
              static_cast<void>(pair[beyond]);
            }),
            "utl: [] beyond the end\r\n");
}

TEST(MisuseTest, RightAccessIsNotReported) {
  EXPECT_EQ(Reported([] {
              utl::array<int, 2> pair = {{1, 2}};
              static_cast<void>(pair[1]);
              const utl::Result<int> ok = 1;
              static_cast<void>(ok.value());
              const utl::optional<int> found = 2;
              static_cast<void>(*found);
            }),
            "");
}
#endif

}  // namespace
}  // namespace utl_test
