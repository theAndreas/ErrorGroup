# Utl

A small standard library for Arduino on the AVR. avr-gcc comes without the C++ standard library;
Utl brings what a sketch or a library misses most, in the namespace `utl`: in the manner of the
standard where it has an answer, and checked where the standard leaves a call undefined.

- **Errors** that say what went wrong and where it came from, in 3 bytes each: `Error`, `Status`
  and `Result`, with a fluent API after Rust, and groups of errors that any library or sketch adds
  without changing this one.
- **Containers**: `array`, `span` and `make_array`, as in the standard, except that what can go
  out of range returns a `Result` rather than being undefined.

## Installing

Not in the Library Manager yet. Clone it into the `libraries` folder of your sketchbook, under the
name `Utl`:

```bash
git clone https://github.com/theAndreas/Utl ~/Arduino/libraries/Utl
```

It is for the AVR boards (`architectures=avr`); it is built and checked for the Uno.

## Errors

An error is a code within a group, `font: unknown char` or `bus: nack`, and a group is identified
by its address: no one hands out group ids, and a library or a sketch adds a group of its own
without changing this one. It tries out the variant "by its address" of
[MaxMatrix' ErrorHandling.md](https://github.com/theAndreas/MaxMatrix/blob/main/docu/ErrorHandling.md).

### Using them

```cpp
#include <Utl.h>

// A function with nothing else to return returns a Status: ok, or an error.
utl::Status SetChar(char c) {
  if (c < ' ') {
    return FontError::kUnknownChar;  // a code of a group is an error
  }
  return utl::Status::Ok();
}

// A function with a value returns a Result: the value, or an error.
utl::Result<char> Receive(bool acknowledged) {
  if (!acknowledged) {
    return BusError::kNack;
  }
  return 'A';
}

// The first error ends the chain.
const utl::Status status =
    Receive(false).AndThen([](char c) { return SetChar(c); });
if (status == BusError::kNack) { ... }
if (const utl::Error* error = status.error()) {
  Serial.print(error->group().Name());  // "bus"
  Serial.print(": ");
}
Serial.println(status.Message());  // "nack", or "ok"; from flash
```

A sketch includes `<Utl.h>`, and before any header of its own that includes `<utl/...>`:
arduino-cli finds a library only by a header directly in its `src/`, going through the includes
in order, and only once it has found it does `<utl/...>` work.

### Defining a group

A group is an enum class of its codes, a class derived from `utl::ErrorGroup` with the name and
the texts, one object of that class, and a `GetErrorGroup()` beside the enum. From
`examples/Basics`:

```cpp
// font_error.h
enum class FontError : uint8_t {
  kUnknownChar,
  kNullText,
};

class FontErrorGroup : public utl::ErrorGroup {
 public:
  constexpr FontErrorGroup() = default;
  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

extern const FontErrorGroup kFontErrorGroup;  // its address is the group

inline const utl::ErrorGroup& GetErrorGroup(FontError) {
  return kFontErrorGroup;
}
```

```cpp
// font_error.cpp
const char kUnknownCharText[] PROGMEM = "unknown char";
const char kNullTextText[] PROGMEM = "null text";

// In the order of FontError.
constexpr auto kMessages PROGMEM =
    utl::make_array(kUnknownCharText, kNullTextText);

constexpr FontErrorGroup kFontErrorGroup{};

const __FlashStringHelper* FontErrorGroup::Name() const { return F("font"); }

const __FlashStringHelper* FontErrorGroup::Message(uint8_t code) const {
  return LookUpMessage(kMessages, code);  // "unknown code" beyond the table
}
```

- **One object, defined in one `.cpp`**, and declared `extern` in the header. A `const` object
  defined in a header gets a copy in every file that includes it, each at an address of its own,
  and errors made in different files would not compare equal.
- **`GetErrorGroup()` lies beside the enum**, in its namespace, where argument-dependent lookup
  finds it. It is what lets a code convert to an `Error`; an enum without one does not.
- **The texts lie in a `constexpr` table in `PROGMEM`.** `constexpr`, so that the table is
  complete when compiled: one initialized at start-up would be written to flash, which fails
  without a word. A group has at most 255 texts, as a code is a `uint8_t`; `LookUpMessage()`
  checks that when compiled.
- **Codes of different groups may overlap**: two errors are equal only if group and code are.

### The types

One class per file in `src/utl/`:

- `Error` (`error.h`): always an error, a group and a code; there is no `Error()`. A code of a
  group converts to it implicitly, so `Error e{FontError::kUnknownChar}` and
  `error == FontError::kUnknownChar` need no function around the code; no other type converts.
- `Status` (`status.h`): what a function returns that has nothing else to return, ok or an
  `Error`, as Rust's `Result<(), E>` or `absl::Status`. `Status::Ok()` is ok, a code or an `Error`
  converts to a `Status`, and `error()` gives the `Error` or null, so that no one gets hold
  of an `Error` that is none. Still 3 bytes: inside, ok is an `Error` with a null group, which
  only `Status` can make.
- `Result<T>` (`result.h`): a value or an `Error`, the value and a `Status` side by side rather
  than in a union; `error()` as in `Status`, and `status()` to compare or print.
- `Result<void>` (`result.h`): a `Status`, for code that is generic in `T`. It is one, by
  inheritance, and converts back; `Map()` with a function that returns nothing gives one, and
  its own `Map()` makes a value from "ok". A sketch's function returns a `Status`.
- `ErrorGroup` (`error_group.h`): the base of a group, with `Name()` and `Message()`.

The fluent functions follow Rust's `Result`, on `Status` and `Result`: `AndThen()` goes on while
there is no error, `Map()` changes the value, `OrElse()` recovers from an error and gets an
`Error` that is one, `ValueOr()` gives a fallback. None calls its function when it has nothing
to give it. A chain of three steps costs 14 bytes of flash more than the same `if`s written out,
and no RAM: the lambdas are inlined.

## Containers

```cpp
utl::array<uint8_t, 3> rows = {{0x18, 0x3c, 0x18}};
for (const uint8_t row : rows) { ... }
rows.at(5).ValueOr(0);  // 0, as 5 lies beyond the end: at() returns a Result

// A span takes a C array, a utl::array and another span alike.
uint16_t Sum(utl::span<const uint8_t> values);
Sum(rows);
utl::span<uint8_t>(rows).subspan(1).Map(Sum);  // a Result: subspan() can fail

constexpr auto kPins = utl::make_array(uint8_t{2}, uint8_t{3});  // utl::array<uint8_t, 2>
```

- `array<T, N>` (`array.h`) and `span<T>` (`span.h`): `std::array`, which avr-gcc lacks, and
  `std::span` from C++20, with the STL names Google allows for an STL-like entity. What the
  standard leaves undefined or throws on beyond the end returns a `Result` with
  `IndexError::kOutOfRange` instead: `at()`, and `first()`, `last()`, `subspan()` of a span.
  `operator[]` stays unchecked. The group `IndexError` costs its vtable and 2 bytes only in a
  sketch that calls one of these.
- `make_array(...)` (`array.h`): an array whose type and size come from its arguments, what
  `std::array a = {...}` deduces from C++17 on. It refuses arguments that would narrow, although
  Arduino compiles with `-fpermissive -Wno-error=narrowing`.

## Examples

- `Basics`: two groups, a `Status` and a `Result` chained, and the error printed on `Serial`.
- `SelfCheck`: checks the library and prints a line per check, then the number that failed.
  `tools/self_check.sh` runs it in simavr and compares with `SelfCheck.expected`.

## vtables in flash

`Name()` and `Message()` are virtual. On the Uno, avr-gcc keeps every vtable in RAM: it reads data
with `LD` from the data space, and the linker copies all of `.rodata` into RAM at start-up. The
texts themselves lie in `PROGMEM`.

The GCC plugin [avr-flash-vtbl](https://github.com/jcmvbkbc/avr-flash-vtbl) puts the vtables into
flash instead: it marks the vtable types `__flash`, and a virtual call then reads its function
pointer with `LPM`. A copy of it, at upstream commit `fbc6ed9`, lies in `tools/avr-flash-vtbl/`
under its own licence, the GPL v2; it is a tool of the build and links nothing into a sketch.

| Example | Flash, vtables in RAM | Flash, in flash | RAM, vtables in RAM | RAM, in flash |
|---|---|---|---|---|
| Basics | 2368 | 2370 | 208 | 174 |
| SelfCheck | 6664 | 6666 | 221 | 179 |

`arduino:avr:uno`, Arduino AVR Boards 1.8.8 (avr-gcc 7.3.0). In `Basics` the 34 bytes are the
vtables of its two groups, 8 bytes each, and of `HardwareSerial`, 18; `SelfCheck` saves 42, as
`at()` brings in a third group, `IndexError`. The group objects keep their 2 bytes of vptr in RAM,
as their address is the group.

**Every C++ file of a sketch has to be compiled with the plugin**, the Arduino core included. A
file compiled without it reads a vtable in flash from RAM and jumps anywhere; the linker does not
notice. `tools/compile_examples.sh` therefore fails when a vtable of the flash build lies in RAM.
The library works without the plugin too, it only costs the RAM.

The Arduino IDE passes no flags per sketch. To use the plugin there, add to the
`platform.local.txt` beside the `platform.txt` of the AVR core:

```text
compiler.cpp.extra_flags=-fplugin=/full/path/to/avr-flash-vtbl.so
```

A plugin loads only into the `cc1plus` it was built for: build it again after an update of the AVR
core.

## Build and check

Needs `arduino-cli` with `arduino:avr`, a host `g++` for the plugin, `simavr`, `uv` and `jq`.

```bash
tools/build_plugin.sh       # build/avr-flash-vtbl.so
tools/self_check.sh         # examples/SelfCheck in simavr, with and without the plugin
tools/compile_examples.sh   # every example both ways, fails on a warning; prints the table above
tools/lint.sh [--fix]       # clang-format, cpplint, shellcheck, actionlint; --fix formats C++
tools/tidy.sh               # clang-tidy on the library and the examples, with the flags of the Uno
```

CI (`.github/workflows/ci.yml`) runs the same scripts on every pull request, and `arduino-lint`
on top; the table of flash and RAM lands in the summary of the job "Examples on arduino:avr:uno".

The code follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html),
the scripts the [Shell Style Guide](https://google.github.io/styleguide/shellguide.html).
Three exceptions, all forced by Arduino: sources end in `.cpp`, as `arduino-cli` compiles no
`.cc`; the header `Utl.h` is named after the library, as Arduino expects; and a sketch
includes its own headers without a directory (see `examples/CPPLINT.cfg`).

`tools/lint.sh` and `tools/tidy.sh` fetch their tools from PyPI through `uvx`, at fixed versions, so
that nothing has to be installed. `.clang-tidy` turns on broad groups of checks and turns off, each
with its reason, what contradicts the Google style or cannot be had on the AVR.

Nothing has run on a board yet; `SelfCheck` should print the same there as in simavr.

## License

GPL-3.0-or-later, see `LICENSE`. The plugin in `tools/avr-flash-vtbl/` is under its own licence,
the GPL v2, in its `COPYING`.
