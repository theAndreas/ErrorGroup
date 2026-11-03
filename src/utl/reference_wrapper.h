// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A reference that copies: std::reference_wrapper, ref() and cref(), for
// gnu++11, without the call through operator().

#ifndef UTL_SRC_UTL_REFERENCE_WRAPPER_H_
#define UTL_SRC_UTL_REFERENCE_WRAPPER_H_

#include "utl/internal/type_traits.h"

namespace utl {

// A reference to a T that can be copied and assigned, as
// std::reference_wrapper, with its names, as Google allows for an STL-like
// entity. Inside a pointer, 2 bytes on the Uno, but one that is never null
// and owns nothing: for a member that refers to an object another owns, where
// a T& would make the class neither copyable nor assignable, and a T* would
// leave open whether it may be null and who deletes it.
//
//   class Panel {
//    public:
//     explicit Panel(Display& display) : display_(display) {}
//     void Clear() { display_.get().Clear(); }
//
//    private:
//     utl::reference_wrapper<Display> display_;
//   };
//
// It binds an lvalue only, as std::reference_wrapper: a temporary would be
// gone before the first use. There is no reference_wrapper(), so a class
// with one as a member has no default constructor either, unless it binds
// the member to something; a reference that may be absent is an optional of
// a reference instead.
//
// Assigning one rebinds it, as std::reference_wrapper does: `a = b` makes `a`
// refer to what `b` refers to and leaves both objects as they are. To write
// through it, use get().
//
// Left out: operator(), which calls a function it refers to as std::invoke.
// That is the one costly part, and a member that refers to an object needs
// none of it.
template <typename T>
class reference_wrapper {
 public:
  using type = T;

  // Implicit, as in std::reference_wrapper, so that a utl::array of them is
  // written as the objects they refer to, `{{red, green}}`. An lvalue of a
  // class derived from T binds as well. __builtin_addressof, what
  // std::addressof is built on, takes the address even of a T that overloads
  // unary &.
  constexpr reference_wrapper(T& object)  // NOLINT(runtime/explicit)
      : pointer_(__builtin_addressof(object)) {}

  // No temporary: it would be gone before the first use. Deleted rather than
  // left out, as an rvalue would otherwise bind to T const& above when T is
  // const; and not explicit, as `reference_wrapper<int const> r = 5` would
  // then pass it by and bind the 5 after all.
  reference_wrapper(T&&) = delete;  // NOLINT(runtime/explicit)

  // One of U that is also one of T: of int to one of int const, of Derived
  // to one of Base. Implicit, as in std::reference_wrapper, and only where a
  // U* converts to a T*, so never of long to int.
  template <typename U, typename = decltype(internal::ConvertTo<T*>(
                            internal::DeclVal<U*>()))>
  constexpr reference_wrapper(  // NOLINT(runtime/explicit)
      reference_wrapper<U> const& other)
      : pointer_(__builtin_addressof(other.get())) {}

  constexpr T& get() const { return *pointer_; }

  // Implicit, as in std::reference_wrapper, so that it passes as a T& to a
  // function that takes one.
  constexpr operator T&() const { return *pointer_; }

 private:
  T* pointer_;
};

// A reference_wrapper of `object`, its type taken from the argument: what
// C++17 deduces from `reference_wrapper r = object`, which gnu++11 cannot.
template <typename T>
constexpr reference_wrapper<T> ref(T& object) {
  return reference_wrapper<T>(object);
}

// Of a reference_wrapper, the same one, not one of a reference_wrapper.
template <typename T>
constexpr reference_wrapper<T> ref(reference_wrapper<T> wrapper) {
  return wrapper;
}

// No temporary, as for the constructor.
template <typename T>
void ref(T const&&) = delete;

// As ref(), of T const: a reference that cannot write.
template <typename T>
constexpr reference_wrapper<T const> cref(T const& object) {
  return reference_wrapper<T const>(object);
}

template <typename T>
constexpr reference_wrapper<T const> cref(reference_wrapper<T> wrapper) {
  return reference_wrapper<T const>(wrapper.get());
}

template <typename T>
void cref(T const&&) = delete;

}  // namespace utl

#endif  // UTL_SRC_UTL_REFERENCE_WRAPPER_H_
