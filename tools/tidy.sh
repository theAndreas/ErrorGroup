#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs clang-tidy on the library and the examples, with the flags arduino-cli
# compiles them with for the Uno, and the checks in .clang-tidy. With
# --database-only it writes the compilation database as built by default and
# stops: clangd reads it for the examples, see .clangd. clang-tidy comes from
# PyPI through uvx, at a fixed version, like the tools of lint.sh.
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

# Writes <dir>/compile_commands.json for the files given, with the flags of
# arduino_flags() and the defines in `defines`, a JSON array.
# Arguments:
#   dir, flags, defines, then the files
write_database() {
  local dir="$1"
  local flags="$2"
  local defines="$3"
  shift 3
  mkdir -p "${dir}"
  printf '%s\n' "$@" | jq -R . | jq -s \
    --arg dir "${ROOT_DIR}" --argjson flags "${flags}" \
    --argjson defines "${defines}" '
    map({
      directory: $dir,
      file: .,
      arguments: ($flags + $defines
        + (if endswith(".ino") then ["-x", "c++", "-include", "Arduino.h"]
           else [] end)
        + [.])
    })' > "${dir}/compile_commands.json"
}

main() {
  local database_only=0
  if [[ "${1:-}" == "--database-only" ]]; then
    database_only=1
  elif [[ $# -gt 0 ]]; then
    err "Usage: $0 [--database-only]"
    exit 2
  fi

  cd "${ROOT_DIR}"
  mkdir -p "${TIDY_DIR}"
  local sources
  mapfile -t sources < <(git ls-files --cached --others --exclude-standard \
    'src/*' 'examples/*' | grep -E '\.(cpp|ino)$')
  local flags
  flags="$(arduino_flags)"

  # As built by default, and with misuse checked: each takes other branches of
  # #if.
  local failed=0
  write_database "${TIDY_DIR}/default" "${flags}" '[]' "${sources[@]}"
  if ((database_only)); then
    echo "Wrote ${TIDY_DIR}/default/compile_commands.json"
    exit 0
  fi
  echo "clang-tidy, as built by default:"
  uvx --from "${CLANG_TIDY}" clang-tidy -p "${TIDY_DIR}/default" --quiet \
    "${sources[@]}" || failed=1
  write_database "${TIDY_DIR}/checked" "${flags}" \
    '["-DUTL_CHECK_ACCESS=1"]' "${sources[@]}"
  echo "clang-tidy, with misuse checked:"
  uvx --from "${CLANG_TIDY}" clang-tidy -p "${TIDY_DIR}/checked" --quiet \
    "${sources[@]}" || failed=1

  # The tests on the host, against the mock, with the database CMake writes.
  # Only what lies in test/: the tests are C++17, and the headers of the
  # library, checked above as the gnu++11 they are, would be held to C++17.
  # Not the mock in test/mock/, which stands in for the Arduino core and
  # follows it, not the Google style.
  local tests
  mapfile -t tests < <(git ls-files --cached --others --exclude-standard \
    'test/*' ':!test/mock/*' | grep -E '\.cpp$')
  cmake --preset default >/dev/null
  echo "clang-tidy, the tests on the host:"
  uvx --from "${CLANG_TIDY}" clang-tidy -p "${BUILD_DIR}/host" --quiet \
    --header-filter='(^|/)test/' \
    "${tests[@]}" || failed=1
  exit "${failed}"
}

main "$@"
