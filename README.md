# ErrorGroup

Errors for Arduino that carry the group they come from. An `Error` is a pointer to its
`ErrorGroup` and a code within it, 3 bytes on the Uno. A group is one object per program, and its
address is what makes it unique: no one hands out group ids, and a component adds a group of its
own without changing the library. It is the variant "by its address" of
[MaxMatrix' ErrorHandling.md](https://github.com/theAndreas/MaxMatrix/blob/main/docu/ErrorHandling.md),
tried out on its own.

```cpp
#include <ErrorGroup.h>

utl::Status SetChar(char c) {
  if (c < ' ') {
    return FontError::kUnknownChar;  // a code of a group is an error
  }
  return utl::Status::Ok();
}

utl::Result<uint8_t> Receive() {
  if (!acknowledged) {
    return BusError::kNack;
  }
  return received;
}

const utl::Status status = Receive().AndThen([](uint8_t c) { return SetChar(c); });
if (status == FontError::kUnknownChar) { ... }
if (const utl::Error* error = status.error()) {
  Serial.print(error->group().Name());  // "font"
}
Serial.println(status.Message());  // "unknown char", or "ok"; from flash
```

Everything lies in the namespace `utl`, one class per file in `src/utl/`:

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
- `array<T, N>` (`array.h`) and `span<T>` (`span.h`): `std::array`, which avr-gcc lacks, and
  `std::span` from C++20, with the STL names Google allows for an STL-like entity. What the
  standard leaves undefined or throws on beyond the end returns a `Result` with
  `IndexError::kOutOfRange` instead: `at()`, and `first()`, `last()`, `subspan()` of a span.
  `operator[]` stays unchecked. The group `IndexError` costs its vtable and 2 bytes only in a
  sketch that calls one of these.
- `make_array(...)` (`array.h`): an array whose type and size come from its arguments, what
  `std::array a = {...}` deduces from C++17 on. It refuses arguments that would narrow, although
  Arduino compiles with `-fpermissive -Wno-error=narrowing`.

The fluent functions follow Rust's `Result`, on `Status` and `Result`: `AndThen()` goes on while
there is no error, `Map()` changes the value, `OrElse()` recovers from an error and gets an
`Error` that is one, `ValueOr()` gives a fallback. None calls its function when it has nothing
to give it. A chain of three steps costs 14 bytes of flash more than the same `if`s written out,
and no RAM: the lambdas are inlined.

A sketch includes `<ErrorGroup.h>`: arduino-cli finds a library only by a header directly in its
`src/`, and once found, `<utl/...>` works in every file of the sketch.

A group is a class derived from `ErrorGroup`, an enum class of its codes, one object defined in one
`.cpp` file, and a `GetErrorGroup()` beside the enum; `examples/Basics` defines two. Its texts lie
in a `constexpr` `utl::array` in `PROGMEM`, made with `make_array()`, which `LookUpMessage()`
reads.

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
`.cc`; the header `ErrorGroup.h` is named after the library, as Arduino expects; and a sketch
includes its own headers without a directory (see `examples/CPPLINT.cfg`).

`tools/lint.sh` and `tools/tidy.sh` fetch their tools from PyPI through `uvx`, at fixed versions, so
that nothing has to be installed. `.clang-tidy` turns on broad groups of checks and turns off, each
with its reason, what contradicts the Google style or cannot be had on the AVR.

Nothing has run on a board yet; `SelfCheck` prints the same there as in simavr.
