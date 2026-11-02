#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs clang-tidy on the library and the examples, with the flags arduino-cli
# compiles them with for the Uno, and the checks in .clang-tidy. clang-tidy
# comes from PyPI through uvx, at a fixed version, like the tools of lint.sh.
#
# arduino-cli compiles copies of a sketch's files in its build folder, and a
# finding there would point to the copy. So the flags are taken from its
# compilation database, and a database of our own names the files in the
# repository; an .ino is compiled as C++ with Arduino.h included first, which
# is what arduino-cli adds to it.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly CLANG_TIDY="clang-tidy==23.1.4"
readonly TIDY_DIR="${BUILD_DIR}/tidy"

# Prints, as a JSON array, the arguments arduino-cli compiles a file of the
# sketch Basics with, without the file, its output and -w, and changed for
# clang:
# - The include folders outside the repository, the core's and the board's,
#   become -isystem, so that nothing is reported from headers we do not own.
# - -Wall -Wextra, so that clang's own warnings are reported as well.
# - -Wno-unknown-attributes: clang does not know avr-gcc's attribute
#   __progmem__, which PROGMEM stands for, and takes the tables as in RAM.
#   That changes nothing clang-tidy checks.
arduino_flags() {
  arduino-cli compile --fqbn "${FQBN}" --library "${ROOT_DIR}" \
    --only-compilation-database --build-path "${TIDY_DIR}/arduino" \
    "${ROOT_DIR}/examples/Basics" >/dev/null
  jq --arg root "${ROOT_DIR}" '[.[] | select(.file | endswith(".ino.cpp"))][0]
    | .file as $file
    | .arguments
    | del(.[index("-o"), index("-o") + 1])
    | map(select(. != $file and . != "-w"))
    | map(if startswith("-I") and (startswith("-I" + $root) | not)
          then "-isystem" + .[2:] else . end)
    + ["-Wall", "-Wextra", "-Wno-unknown-attributes"]' \
    "${TIDY_DIR}/arduino/compile_commands.json"
}

# Writes TIDY_DIR/compile_commands.json for the files given.
write_database() {
  local flags
  flags="$(arduino_flags)"
  printf '%s\n' "$@" | jq -R . | jq -s \
    --arg dir "${ROOT_DIR}" --argjson flags "${flags}" '
    map({
      directory: $dir,
      file: .,
      arguments: ($flags
        + (if endswith(".ino") then ["-x", "c++", "-include", "Arduino.h"]
           else [] end)
        + [.])
    })' > "${TIDY_DIR}/compile_commands.json"
}

main() {
  cd "${ROOT_DIR}"
  mkdir -p "${TIDY_DIR}"
  local sources
  mapfile -t sources < <(git ls-files --cached --others --exclude-standard \
    'src/*' 'examples/*' | grep -E '\.(cpp|ino)$')

  write_database "${sources[@]}"
  uvx --from "${CLANG_TIDY}" clang-tidy -p "${TIDY_DIR}" --quiet \
    "${sources[@]}"
}

main "$@"
