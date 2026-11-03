// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The few pieces of <utility> and <type_traits> that utl needs: avr-gcc comes
// without a C++ standard library.

#ifndef UTL_SRC_UTL_INTERNAL_TYPE_TRAITS_H_
#define UTL_SRC_UTL_INTERNAL_TYPE_TRAITS_H_

namespace utl {
namespace internal {

// std::declval. Declared only: it names a value of type T in decltype, where
// nothing is evaluated.
template <typename T>
T&& DeclVal();

// Declared only, like DeclVal: a call compiles if its argument converts to To
// implicitly.
template <typename To>
void ConvertTo(To);

// std::void_t: void, if every type in Ts is valid. Through a struct, as an
// alias template alone may ignore arguments it does not use (CWG 1558).
template <typename... Ts>
struct VoidImpl {
  using Type = void;
};
template <typename... Ts>
using Void = typename VoidImpl<Ts...>::Type;

// std::remove_cv_t.
template <typename T>
struct RemoveCvImpl {
  using Type = T;
};
template <typename T>
struct RemoveCvImpl<T const> {
  using Type = T;
};
template <typename T>
struct RemoveCvImpl<T volatile> {
  using Type = T;
};
template <typename T>
struct RemoveCvImpl<T const volatile> {
  using Type = T;
};
template <typename T>
using RemoveCv = typename RemoveCvImpl<T>::Type;

// A pointer to an array of T of unknown size: the type the test below works
// with, not an array that wants to be a utl::array.
template <typename T>
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
using ArrayPointer = T (*)[];

// Names a type only if an array of From may be seen as an array of To, the
// test std::span makes: it adds const or volatile, as int to const int, but
// it never turns Derived into Base, whose elements would lie at other
// distances, nor long into int.
template <typename From, typename To>
using ArrayConvertible =
    decltype(ConvertTo<ArrayPointer<To>>(DeclVal<ArrayPointer<From>>()));

}  // namespace internal
}  // namespace utl

#endif  // UTL_SRC_UTL_INTERNAL_TYPE_TRAITS_H_
