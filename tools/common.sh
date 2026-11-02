#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Shared by the scripts in tools/: where things lie, and how an example is
# compiled in either variant, "ram" (vtables in RAM, the default of avr-gcc)
# or "flash" (vtables in flash, with the plugin avr-flash-vtbl).

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly ROOT_DIR
readonly BUILD_DIR="${ROOT_DIR}/build"
readonly PLUGIN="${BUILD_DIR}/avr-flash-vtbl.so"
readonly FQBN="arduino:avr:uno"
# shellcheck disable=SC2034  # used by the scripts that source this file
readonly VARIANTS=(ram flash)

err() {
  echo "$*" >&2
}

# Prints the name of every example, one per line.
list_examples() {
  local dir
  for dir in "${ROOT_DIR}"/examples/*/; do
    basename "${dir}"
  done
}

# Prints the bin directory of the avr-gcc that arduino-cli uses for FQBN.
avr_gcc_bin_dir() {
  arduino-cli compile --fqbn "${FQBN}" --show-properties \
      "${ROOT_DIR}/examples/Basics" 2>/dev/null \
    | sed -n 's/^compiler\.path=//p'
}

# Compiles an example in a variant into BUILD_DIR/<variant>/<example> and
# prints what arduino-cli prints.
# Arguments:
#   variant: ram or flash
#   example: the name of a folder in examples/
compile_example() {
  local variant="$1"
  local example="$2"
  local args=(
    --fqbn "${FQBN}"
    --library "${ROOT_DIR}"
    --warnings all
    --build-path "${BUILD_DIR}/${variant}/${example}"
  )
  if [[ "${variant}" == "flash" ]]; then
    if [[ ! -f "${PLUGIN}" ]]; then
      err "${PLUGIN} is missing; run tools/build_plugin.sh first."
      return 1
    fi
    # extra_flags reach every C++ file of the build, the core included. A file
    # compiled without the plugin would read a vtable in flash from RAM.
    args+=(--build-property "compiler.cpp.extra_flags=-fplugin=${PLUGIN}")
  fi
  arduino-cli compile "${args[@]}" "${ROOT_DIR}/examples/${example}" 2>&1
}

# Prints the path of the ELF file that compile_example made.
elf_of() {
  local variant="$1"
  local example="$2"
  echo "${BUILD_DIR}/${variant}/${example}/${example}.ino.elf"
}
