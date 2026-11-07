// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief std::invoke_r: how function_ref and inplace_function call what they
 *        hold; not part of the API.
 */

#ifndef UTL_SRC_UTL_INTERNAL_INVOKE_H_
#define UTL_SRC_UTL_INTERNAL_INVOKE_H_

#include "utl/internal/type_traits.h"

namespace utl {
namespace internal {

/*
 * std::invoke, for what a template argument can be: a function, called with
 * all the arguments, or a pointer to a member function, called on the first,
 * an object of its class or a pointer to one, with the others. Not for a
 * pointer to a data member, which nothing here calls. f is taken by value,
 * a constant that the call is inlined with.
 */
template <typename F>
struct Invoker {
  template <typename... Args>
  static auto Call(F f, Args&&... args)
      -> decltype(f(static_cast<Args&&>(args)...)) {
    return f(static_cast<Args&&>(args)...);
  }
};
template <typename M, typename C>
struct Invoker<M C::*> {
  template <typename T, typename... Args>
  static auto Call(M C::* f, T&& object, Args&&... args)
      -> decltype((static_cast<T&&>(object).*f)(static_cast<Args&&>(args)...)) {
    return (static_cast<T&&>(object).*f)(static_cast<Args&&>(args)...);
  }
  template <typename T, typename... Args>
  static auto Call(M C::* f, T* object, Args&&... args)
      -> decltype((object->*f)(static_cast<Args&&>(args)...)) {
    return (object->*f)(static_cast<Args&&>(args)...);
  }
};

// The test of std::is_invocable_r for what Invoker calls: void if f of type
// F can be invoked with arguments of Args and its result converts to R, or R
// is void; no type if not.
template <typename R, typename F, typename... Args>
using InvocableAs =
    typename ResultConvertsTo<R>::template Type<decltype(Invoker<F>::Call(
        DeclVal<F>(), DeclVal<Args>()...))>;

// Calls `f` with `args` and returns its result as an R, converted
// implicitly, as a static_cast<R> would also take an explicit constructor.
template <typename R>
struct CallReturning {
  template <typename F, typename... Args>
  static R Call(F& f, Args&&... args) {
    return f(static_cast<Args&&>(args)...);
  }

  template <typename F, typename... Args>
  static R Invoke(F f, Args&&... args) {
    return Invoker<F>::Call(f, static_cast<Args&&>(args)...);
  }
};

// For R void, the result is dropped, whatever it is.
template <>
struct CallReturning<void> {
  template <typename F, typename... Args>
  static void Call(F& f, Args&&... args) {
    f(static_cast<Args&&>(args)...);
  }

  template <typename F, typename... Args>
  static void Invoke(F f, Args&&... args) {
    Invoker<F>::Call(f, static_cast<Args&&>(args)...);
  }
};

}  // namespace internal
}  // namespace utl

#endif  // UTL_SRC_UTL_INTERNAL_INVOKE_H_
