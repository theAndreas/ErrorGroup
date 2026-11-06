// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// function_ref: a reference to something to call.

#include <gtest/gtest.h>
#include <utl/function_ref.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace utl_test {
namespace {

using utl::function_ref;

int Twice(int value) { return 2 * value; }
void Nothing() {}
int calls = 0;
void CountCall() { ++calls; }

// Calls f once per value, the use it is made for: a parameter, which takes a
// function, a lambda or an object without being a template.
int Sum(function_ref<int(int)> f) {
  int sum = 0;
  for (int value = 1; value <= 3; ++value) {
    sum += f(value);
  }
  return sum;
}

// Which operator() is called tells whether f was const.
struct Overloaded {
  int operator()() { return 1; }
  int operator()() const { return 2; }
};

// Callable, but only with an int.
struct TakesInt {
  void operator()(int /*value*/) const {}
};

// A result that converts to an R only explicitly.
struct Explicit {
  explicit Explicit(int /*value*/) {}
};

struct NotCallable {};

// A callable that overloads unary &, which must not be taken for its
// address.
struct AddressHidden {
  int operator()() const { return 5; }
  // NOLINTNEXTLINE(google-runtime-operator)
  void operator&() const = delete;  // NOLINT(runtime/operator)
};

// Two pointers, and nothing it owns: copied as they are, without a destructor
// of its own.
static_assert(sizeof(function_ref<void()>) == 2 * sizeof(void*));
static_assert(std::is_trivially_copyable_v<function_ref<void()>>);
static_assert(std::is_trivially_destructible_v<function_ref<void()>>);

// Never empty.
static_assert(!std::is_default_constructible_v<function_ref<void()>>);
static_assert(!std::is_constructible_v<function_ref<void()>, std::nullptr_t>);

// Binds what can be called with Args and returns what converts to R.
static_assert(std::is_convertible_v<int (*)(int), function_ref<int(int)>>);
static_assert(std::is_convertible_v<int (&)(int), function_ref<int(int)>>);
static_assert(std::is_convertible_v<Overloaded&, function_ref<int()>>);
static_assert(
    std::is_convertible_v<int (*)(int), function_ref<int64_t(int16_t)>>);
static_assert(std::is_convertible_v<int (*)(int), function_ref<void(int)>>);
static_assert(!std::is_constructible_v<function_ref<void()>, int>);
static_assert(!std::is_constructible_v<function_ref<void()>, NotCallable>);
static_assert(!std::is_constructible_v<function_ref<void()>, TakesInt>);
static_assert(!std::is_constructible_v<function_ref<void(int*)>, TakesInt>);
static_assert(!std::is_constructible_v<function_ref<int()>, void (*)()>);
static_assert(!std::is_constructible_v<function_ref<Explicit()>, int (*)()>);
static_assert(!std::is_constructible_v<function_ref<int(int)>, int (*)()>);

// Assigned only another function_ref or a function, never a lambda or an
// object, which would be gone before the next call.
static_assert(
    std::is_assignable_v<function_ref<void()>&, function_ref<void()>>);
static_assert(std::is_assignable_v<function_ref<void()>&, void (*)()>);
static_assert(std::is_assignable_v<function_ref<void()>&, void (&)()>);
static_assert(!std::is_assignable_v<function_ref<void()>&, std::nullptr_t>);

TEST(FunctionRefTest, BindsALambdaButIsNeverAssignedOne) {
  auto const nothing = [] {};
  using Lambda = std::remove_const_t<decltype(nothing)>;
  static_assert(std::is_convertible_v<Lambda, function_ref<void()>>);
  static_assert(!std::is_assignable_v<function_ref<void()>&, Lambda>);
  static_assert(!std::is_assignable_v<function_ref<void()>&, Lambda&>);
  function_ref<void()> const f = nothing;
  f();
}

TEST(FunctionRefTest, CallsAFunction) {
  function_ref<int(int)> const by_name = Twice;
  function_ref<int(int)> const by_pointer = &Twice;
  EXPECT_EQ(by_name(3), 6);
  EXPECT_EQ(by_pointer(4), 8);
  EXPECT_EQ(Sum(Twice), 12);
}

TEST(FunctionRefTest, CallsAFunctionOfOtherTypes) {
  function_ref<int64_t(int16_t)> const widened = Twice;
  EXPECT_EQ(widened(int16_t{21}), int64_t{42});
  function_ref<void(int)> const dropped = Twice;  // the result is dropped
  dropped(1);
}

TEST(FunctionRefTest, CallsALambdaWithItsCaptures) {
  int offset = 10;
  auto const add = [&offset](int value) { return value + offset; };
  function_ref<int(int)> const f = add;
  EXPECT_EQ(f(1), 11);
  offset = 20;  // seen, as the lambda is referred to, not copied
  EXPECT_EQ(f(1), 21);
  EXPECT_EQ(Sum([](int value) { return value * value; }), 14);
}

TEST(FunctionRefTest, RefersToTheObjectNotACopy) {
  int count = 0;
  auto counter = [count]() mutable { return ++count; };
  function_ref<int()> const f = counter;
  EXPECT_EQ(f(), 1);
  EXPECT_EQ(f(), 2);
  EXPECT_EQ(counter(), 3);  // the same object, counting on
}

TEST(FunctionRefTest, CallsConstOperatorOfAConstObject) {
  Overloaded mutable_object;
  Overloaded const const_object;
  function_ref<int()> const f = mutable_object;
  function_ref<int()> const g = const_object;
  EXPECT_EQ(f(), 1);
  EXPECT_EQ(g(), 2);
}

TEST(FunctionRefTest, TakesTheAddressPastAnOverloadedAmpersand) {
  AddressHidden const hidden;
  function_ref<int()> const f = hidden;
  EXPECT_EQ(f(), 5);
}

TEST(FunctionRefTest, PassesArgumentsAsTaken) {
  auto const increment = [](int& value) { ++value; };
  function_ref<void(int&)> const f = increment;
  int value = 1;
  f(value);
  EXPECT_EQ(value, 2);

  // Moved through to the callable, never copied.
  auto const take = [](std::unique_ptr<int> owned) { return *owned; };
  function_ref<int(std::unique_ptr<int>)> const g = take;
  EXPECT_EQ(g(std::make_unique<int>(7)), 7);
}

TEST(FunctionRefTest, ReturnsAReference) {
  int target = 1;
  auto const get = [&target]() -> int& { return target; };
  function_ref<int&()> const f = get;
  f() = 5;
  EXPECT_EQ(target, 5);
  EXPECT_EQ(&f(), &target);
}

TEST(FunctionRefTest, CopiesAndRebinds) {
  calls = 0;
  function_ref<void()> f = Nothing;
  function_ref<void()> const copy = f;
  f = CountCall;  // a function, assigned by name
  f();
  copy();  // still Nothing
  EXPECT_EQ(calls, 1);
  auto const twice = [] {
    CountCall();
    CountCall();
  };
  f = function_ref<void()>(twice);
  f();
  EXPECT_EQ(calls, 3);
}

TEST(FunctionRefTest, IsAConstantOfAnObjectWithStaticStorage) {
  static constexpr Overloaded kObject{};
  constexpr function_ref<int()> kF = kObject;
  EXPECT_EQ(kF(), 2);
}

}  // namespace
}  // namespace utl_test
