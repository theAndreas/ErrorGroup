// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Result<T> and Result<void>: a value or an Error, and the fluent functions.

#include <gtest/gtest.h>
#include <utl/error.h>
#include <utl/result.h>
#include <utl/status.h>

#include "test/alpha_error.h"
#include "test/beta_error.h"

namespace utl_test {
namespace {

using utl::Error;
using utl::Result;
using utl::Status;

// Returns `code` for an index below 3, or BetaError::kFirst.
Result<int> Get(int index) {
  if (index >= 3) {
    return BetaError::kFirst;
  }
  return index * 10;
}

TEST(ResultTest, HoldsAValue) {
  const Result<int> result = Get(2);
  EXPECT_TRUE(result.IsOk());
  EXPECT_EQ(result.value(), 20);
  EXPECT_EQ(result.error(), nullptr);
  EXPECT_TRUE(result.status().IsOk());
}

TEST(ResultTest, HoldsAnError) {
  const Result<int> result = Get(3);
  EXPECT_FALSE(result.IsOk());
  ASSERT_NE(result.error(), nullptr);
  EXPECT_EQ(*result.error(), BetaError::kFirst);
  EXPECT_EQ(result.status(), BetaError::kFirst);
  EXPECT_EQ(Result<int>(Error(AlphaError::kFirst)).status(),
            AlphaError::kFirst);
}

TEST(ResultTest, ValueOr) {
  EXPECT_EQ(Get(1).ValueOr(-1), 10);
  EXPECT_EQ(Get(5).ValueOr(-1), -1);
}

TEST(ResultTest, MapChangesTheValue) {
  EXPECT_EQ(Get(2).Map([](int value) { return value + 1; }).ValueOr(0), 21);
  int calls = 0;
  const Result<bool> mapped = Get(5).Map([&calls](int) {
    ++calls;
    return true;
  });
  EXPECT_EQ(mapped.status(), BetaError::kFirst);
  EXPECT_EQ(calls, 0);
}

TEST(ResultTest, AndThenChainsAndStops) {
  EXPECT_EQ(Get(0).AndThen([](int) { return Get(2); }).ValueOr(0), 20);
  int calls = 0;
  const Result<int> broken =
      Get(1).AndThen([](int) { return Get(9); }).AndThen([&calls](int value) {
        ++calls;
        return Get(value);
      });
  EXPECT_EQ(broken.status(), BetaError::kFirst);
  EXPECT_EQ(calls, 0);
  const Status checked = Get(2).AndThen([](int value) {
    return value == 20 ? Status::Ok() : Status(AlphaError::kFirst);
  });
  EXPECT_TRUE(checked.IsOk());
}

TEST(ResultTest, OrElseRecoversOrKeeps) {
  EXPECT_EQ(Get(9).OrElse([](const Error&) { return 7; }).ValueOr(0), 7);
  int calls = 0;
  const Result<int> kept = Get(1).OrElse([&calls](const Error&) {
    ++calls;
    return Result<int>(0);
  });
  EXPECT_EQ(kept.value(), 10);
  EXPECT_EQ(calls, 0);
}

TEST(ResultVoidTest, IsAStatus) {
  const Result<void> ok = Result<void>::Ok();
  EXPECT_TRUE(ok.IsOk());
  EXPECT_EQ(ok, Result<void>());
  const Result<void> failed = AlphaError::kFirst;
  const Status status = failed;
  EXPECT_EQ(status, AlphaError::kFirst);
  EXPECT_EQ(failed.status(), AlphaError::kFirst);
  const Result<void> from_status = Status(BetaError::kFirst);
  EXPECT_EQ(from_status, BetaError::kFirst);
}

TEST(ResultVoidTest, MapOfAFunctionThatReturnsNothing) {
  int stored = 0;
  const Result<void> done =
      Get(2).Map([&stored](int value) { stored = value; });
  EXPECT_TRUE(done.IsOk());
  EXPECT_EQ(stored, 20);
  stored = 0;
  const Result<void> not_done =
      Get(9).Map([&stored](int value) { stored = value; });
  EXPECT_EQ(not_done, BetaError::kFirst);
  EXPECT_EQ(stored, 0);
}

TEST(ResultVoidTest, MapMakesAValue) {
  EXPECT_EQ(Result<void>::Ok().Map([] { return 5; }).ValueOr(0), 5);
  int calls = 0;
  const Result<int> none = Result<void>(AlphaError::kFirst).Map([&calls] {
    ++calls;
    return 5;
  });
  EXPECT_EQ(none.status(), AlphaError::kFirst);
  EXPECT_EQ(calls, 0);
  int printed = 0;
  const Result<void> nothing =
      Result<void>::Ok().Map([&printed] { ++printed; });
  EXPECT_TRUE(nothing.IsOk());
  EXPECT_EQ(printed, 1);
}

}  // namespace
}  // namespace utl_test
