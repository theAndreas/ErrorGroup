# ErrorGroup

Errors for Arduino that carry the group they come from. An `Error` is a pointer to its
`ErrorGroup` and a code within it, 3 bytes on the Uno. A group is one object per program, and its
address is what makes it unique: no one hands out group ids, and a component adds a group of its
own without changing the library. It is the variant "by its address" of
[MaxMatrix' ErrorHandling.md](https://github.com/theAndreas/MaxMatrix/blob/main/docu/ErrorHandling.md),
tried out on its own.

```cpp
Error SetChar(char c) {
  if (c < ' ') {
    return MakeError(FontError::kUnknownChar);
  }
  return Error();  // no error
}

const Error error = SetChar(c);
if (error == FontError::kUnknownChar) { ... }
Serial.println(error.Message());  // "unknown char", from flash
```

A group is a class derived from `ErrorGroup`, an enum of its codes, one object defined in one
`.cpp` file, and a `GetErrorGroup()` beside the enum; `examples/Basics` defines two.

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
| Basics | 2454 | 2456 | 208 | 174 |
| SelfCheck | 2538 | 2540 | 211 | 177 |

`arduino:avr:uno`, Arduino AVR Boards 1.8.8 (avr-gcc 7.3.0). The 34 bytes are the vtables of the
two groups, 8 bytes each, and of `HardwareSerial`, 18. The group objects keep their 2 bytes of
vptr in RAM, as their address is the group.

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

Needs `arduino-cli` with `arduino:avr`, a host `g++` for the plugin, `simavr`, and `uv`.

```bash
tools/build_plugin.sh       # build/avr-flash-vtbl.so
tools/self_check.sh         # examples/SelfCheck in simavr, with and without the plugin
tools/compile_examples.sh   # every example both ways, fails on a warning; prints the table above
tools/lint.sh [--fix]       # clang-format, cpplint, shellcheck; --fix formats the C++ sources
```

The code follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html),
the scripts the [Shell Style Guide](https://google.github.io/styleguide/shellguide.html).
`tools/lint.sh` fetches its tools from PyPI through `uvx`, at fixed versions, so that nothing has to
be installed. Two exceptions, both forced by Arduino: sources end in `.cpp`, as `arduino-cli`
compiles no `.cc`, and an include names a header without its directory (see `CPPLINT.cfg`).

Nothing has run on a board yet; `SelfCheck` prints the same there as in simavr.
