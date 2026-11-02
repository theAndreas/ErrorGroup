// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Error: a group and a code, and how it prints.

#include <gtest/gtest.h>
#include <utl/config.h>
#include <utl/error.h>
#include <utl/index_error.h>
#include <utl/optional_error.h>

#include <string>

#include "test/alpha_error.h"
#include "test/beta_error.h"
#include "test/string_print.h"

namespace utl_test {
namespace {

using utl::Error;

std::string Printed(const Error& error) {
  StringPrint out;
  const size_t count = error.PrintTo(out);
  EXPECT_EQ(count, out.text.size());
  return out.text;
}

// An error knows its group by address and its code.
TEST(ErrorTest, HasGroupAndCode) {
  const Error error = AlphaError::kSecond;
  EXPECT_EQ(&error.group(), &kAlphaErrorGroup);
  EXPECT_EQ(error.code(), 1);
}

// Equal only if group and code are; codes of two groups may overlap.
TEST(ErrorTest, ComparesGroupAndCode) {
  const Error alpha_first = AlphaError::kFirst;
  EXPECT_EQ(alpha_first, AlphaError::kFirst);
  EXPECT_EQ(AlphaError::kFirst, alpha_first);
  EXPECT_NE(alpha_first, AlphaError::kSecond);
  EXPECT_NE(alpha_first, BetaError::kFirst);
  EXPECT_EQ(Error(kBetaErrorGroup, 0), BetaError::kFirst);
}

// The group is its address: an error made in another file is equal.
TEST(ErrorTest, EqualAcrossTranslationUnits) {
  EXPECT_EQ(MakeAlphaSecondElsewhere(), AlphaError::kSecond);
}

TEST(ErrorTest, IsAssignable) {
  Error error = AlphaError::kFirst;
  error = BetaError::kFirst;
  EXPECT_EQ(error, BetaError::kFirst);
}

#if UTL_ERROR_TEXTS
TEST(ErrorTest, SaysWhatWentWrong) {
  EXPECT_STREQ(
      reinterpret_cast<const char*>(Error(AlphaError::kSecond).Message()),
      "alpha second");
  EXPECT_EQ(Printed(BetaError::kFirst), "beta: beta first");
}

// A code beyond a group's table of texts.
TEST(ErrorTest, UnknownCode) {
  EXPECT_EQ(Printed(Error(kBetaErrorGroup, 7)), "beta: unknown code");
}

// The groups of the library itself.
TEST(ErrorTest, LibraryGroupsSayWhatWentWrong) {
  EXPECT_EQ(Printed(utl::IndexError::kOutOfRange), "index: out of range");
  EXPECT_EQ(Printed(utl::OptionalError::kEmpty), "optional: empty");
}
#else
// Without texts an error prints its code, every digit of it.
TEST(ErrorTest, PrintsCodeWithoutTexts) {
  EXPECT_STREQ(
      reinterpret_cast<const char*>(Error(AlphaError::kSecond).Message()), "");
  EXPECT_EQ(Printed(AlphaError::kSecond), "alpha: 1");
  EXPECT_EQ(Printed(Error(kBetaErrorGroup, 10)), "beta: 10");
  EXPECT_EQ(Printed(Error(kBetaErrorGroup, 105)), "beta: 105");
  EXPECT_EQ(Printed(Error(kBetaErrorGroup, 255)), "beta: 255");
}

TEST(ErrorTest, LibraryGroupsPrintCodesWithoutTexts) {
  EXPECT_EQ(Printed(utl::IndexError::kOutOfRange), "index: 0");
  EXPECT_EQ(Printed(utl::OptionalError::kEmpty), "optional: 0");
}
#endif

}  // namespace
}  // namespace utl_test
