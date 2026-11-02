// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Status: ok or an Error, and its fluent functions.

#include <gtest/gtest.h>
#include <utl/config.h>
#include <utl/error.h>
#include <utl/result.h>
#include <utl/status.h>

#include <string>

#include "test/alpha_error.h"
#include "test/beta_error.h"
#include "test/string_print.h"

namespace utl_test {
namespace {

using utl::Error;
using utl::Result;
using utl::Status;

TEST(StatusTest, OkHasNoError) {
  const Status ok = Status::Ok();
  EXPECT_TRUE(ok.IsOk());
  EXPECT_EQ(ok.error(), nullptr);
  EXPECT_EQ(ok, Status());
  EXPECT_STREQ(reinterpret_cast<const char*>(ok.Message()), "ok");
}

TEST(StatusTest, HoldsItsError) {
  const Status status = AlphaError::kSecond;
  EXPECT_FALSE(status.IsOk());
  ASSERT_NE(status.error(), nullptr);
  EXPECT_EQ(*status.error(), AlphaError::kSecond);
  EXPECT_EQ(status, AlphaError::kSecond);
  EXPECT_EQ(status, Error(AlphaError::kSecond));
  EXPECT_EQ(Error(AlphaError::kSecond), status);
  EXPECT_NE(status, BetaError::kFirst);
  EXPECT_NE(status, Status::Ok());
}

TEST(StatusTest, MessageOfAnError) {
#if UTL_ERROR_TEXTS
  EXPECT_STREQ(
      reinterpret_cast<const char*>(Status(BetaError::kFirst).Message()),
      "beta first");
#else
  EXPECT_STREQ(
      reinterpret_cast<const char*>(Status(BetaError::kFirst).Message()), "");
#endif
}

TEST(StatusTest, PrintsOkOrTheError) {
  StringPrint ok;
  EXPECT_EQ(Status::Ok().PrintTo(ok), 2u);
  EXPECT_EQ(ok.text, "ok");
  StringPrint error;
  const size_t count = Status(BetaError::kFirst).PrintTo(error);
  EXPECT_EQ(count, error.text.size());
#if UTL_ERROR_TEXTS
  EXPECT_EQ(error.text, "beta: beta first");
#else
  EXPECT_EQ(error.text, "beta: 0");
#endif
}

TEST(StatusTest, AndThenGoesOnWhenOk) {
  EXPECT_EQ(Status::Ok().AndThen([] { return Status(AlphaError::kSecond); }),
            AlphaError::kSecond);
  EXPECT_EQ(Status::Ok().AndThen([] { return Result<int>(5); }).ValueOr(0), 5);
}

TEST(StatusTest, AndThenStopsAtAnError) {
  int calls = 0;
  const Status stopped = Status(AlphaError::kFirst).AndThen([&calls] {
    ++calls;
    return Status::Ok();
  });
  EXPECT_EQ(stopped, AlphaError::kFirst);
  EXPECT_EQ(calls, 0);
  const Result<int> no_value =
      Status(AlphaError::kFirst).AndThen([] { return Result<int>(5); });
  EXPECT_EQ(no_value.status(), AlphaError::kFirst);
}

TEST(StatusTest, OrElseGetsTheErrorAndRecovers) {
  const Status recovered =
      Status(AlphaError::kFirst).OrElse([](const Error& error) {
        return error == AlphaError::kFirst ? Status::Ok()
                                           : Status(BetaError::kFirst);
      });
  EXPECT_TRUE(recovered.IsOk());
  int calls = 0;
  const Status untouched = Status::Ok().OrElse([&calls](const Error&) {
    ++calls;
    return Status(AlphaError::kFirst);
  });
  EXPECT_TRUE(untouched.IsOk());
  EXPECT_EQ(calls, 0);
}

}  // namespace
}  // namespace utl_test
