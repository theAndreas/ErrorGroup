# Documenting the code

How the code of Utl is documented: which comments are Doxygen blocks, what each block says, and how
the style is checked. The reference built from the blocks is published on GitHub Pages; the style
holds for every header in `src/`.

## Two kinds of comment

- **A Doxygen block `/*! ... */`** documents an entity for its user: what it is, how to use it,
  what it costs. It stands at the declaration, in the header, and ends up in the reference.
- **A plain `//` comment** explains the implementation to the next person who changes it: why it
  is written this way, which alternative was rejected and why. It is not in the reference.

A comment says what the code cannot say for itself. It does not repeat the code.

## The block

```cpp
/*!
 * \brief Returns the element at `index`, or IndexError::kOutOfRange.
 *
 * A copy, as a Result holds a value. To write, check the index and use
 * operator[].
 *
 * \param[in] index The position, from 0.
 * \return The element, or IndexError::kOutOfRange if `index` >= size().
 *
 * \stddiff Returns a Result where std::span::at() throws std::out_of_range.
 */
Result<value_type> at(size_type index) const;
```

- `/*!` on a line of its own, each further line starting with ` * `, and ` */` on a line of its
  own. A block that holds only a brief is one line: `/*! \brief The width, in columns. */`.
- An enum value, a member or a struct field is documented behind it, with `/*!< ... */`, when the
  brief is all it needs and fits on the line. If it does not, a one-line block
  `/*! \brief ... */` stands above it, as a comment wrapped behind code is what cpplint calls a
  "complex multi-line comment".
- Commands start with a backslash, `\brief`, never with `@`.

### The parts, in this order

| Part | Says |
|---|---|
| `\brief` | One sentence, ending in a full stop. Always written out. |
| Description | After an empty line: what the user has to know beyond the brief, in a few short paragraphs. |
| `\tparam` | Each template parameter that the user chooses. |
| `\param[in]`, `[out]`, `[in,out]` | Each parameter, with its direction. |
| `\return` | What comes back. For a `Result`, the value and each error, with when. A one-line block whose brief already says it, "Returns the number of elements.", leaves it out. |
| `\pre` | What the caller must ensure, and what happens if not: undefined, or reported with `UTL_CHECK_ACCESS`. |
| `\note`, `\warning` | What is easy to miss. `\warning` only where a mistake is costly. |
| `\stddiff` | Where it differs from the entity of the standard it follows, and why. |
| `\see` | A related entity, where the link helps. |

A function that returns nothing has no `\return`. Every parameter has its `\param`, even the only one. A part that has nothing to say is left out.

### The brief

- A function: a verb in the third person, saying what it does. "Returns the element at
  `index`.", "Writes `value` into `buffer`.", "Turns on the texts of the group."
- A type: a noun phrase, saying what it is. "A view of chars in RAM.", "A value of T, or none."
- A value or a member: what it holds or means. "The base, 2 to 36."

### The description

- Plain, complete sentences. Present tense, active voice, no "we".
- What the user needs to choose and to use the entity correctly: its cost on the AVR in flash and
  RAM where it matters, its limits, what it does at the edges.
- An example in `\code{.cpp} ... \endcode` where a call is clearer than a sentence. Keep it short
  and make it compile.
- Names in backticks: parameters as `index`, entities as `Result` or `std::span`. Doxygen links
  an entity of Utl by its name; write `Result::Map()` to link a member.

### What is documented

- **Everything a sketch can use** in `src/`: every class, function, member function, data
  member, enum and enum value, type alias, constant and macro, and each header with `\file`.
- **Overloads** that differ only in their parameters share one text: the first is documented,
  the others take it with `\copydoc`, or are grouped with `\overload` and their own `\param`s.
- **`utl::internal`** is not part of the API. A block is welcome but not required. The exception
  is `internal/fluent.h`, whose functions `Status` and `Result` give a sketch: it is documented
  as the API.
- **Helpers a sketch never sees**, in a file that is otherwise documented as the API, are put
  between `/*! \cond */` and `/*! \endcond */`: Doxygen leaves them out, and they keep their
  `//` comments.
- **Source files, tests and examples** keep `//` comments.

### A file

Every header starts with the licence, as `//` lines that tools such as REUSE read, and then a
block for the file:

```cpp
// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief std::span, from C++20, for gnu++11.
 *
 * What can go out of range returns a Result instead of being undefined.
 */
```

## Checking

- **`tools/docs.sh`** builds the reference with Doxygen and fails on every warning: a block
  Doxygen cannot read, a parameter without `\param` or a `\param` without parameter, an entity
  of the API without a block. Until every header is converted, gaps in the headers not yet in
  its list `CONVERTED` are counted, not failed.
- **clang's `-Wdocumentation`**, through clang-tidy and clangd, checks each block against the
  declaration it stands at: a `\param` whose name the function does not have, a `\return` on a
  function that returns nothing, an unknown command. clangd shows these in the editor as you
  type.
- **clang-format** aligns the blocks; `tools/lint.sh --fix` applies it.

CI runs `tools/docs.sh` on every pull request and publishes the reference from `main`.

## Writing a block

The dev container has the extension "Doxygen Documentation Generator". Type `/*!` above a
declaration and press Enter: it writes the skeleton of this style, with a `\param` for each
parameter and a `\return` for a function that returns a value. The text is yours to write.
