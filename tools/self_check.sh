#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs examples/SelfCheck in simavr, in both variants, and compares what it
# prints with SelfCheck.expected.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly EXAMPLE="SelfCheck"
readonly EXPECTED="${ROOT_DIR}/examples/${EXAMPLE}/${EXAMPLE}.expected"

# Prints what the sketch sent on the UART. simavr writes it to stderr, a line
# in color with CR LF shown as "..", and its own messages to stdout.
run_in_simavr() {
  local elf="$1"
  timeout 10 simavr -m atmega328p -f 16000000 "${elf}" 2>&1 >/dev/null \
    | sed -e 's/\x1b\[[0-9;]*m//g' -e 's/\.\.$//' \
    | grep -v '^$'
}

main() {
  local failed=0
  local variant
  local output
  local actual

  for variant in "${VARIANTS[@]}"; do
    if ! output="$(compile_example "${variant}" "${EXAMPLE}")"; then
      err "${EXAMPLE} (${variant}) does not compile:"
      err "${output}"
      failed=1
      continue
    fi
    actual="$(run_in_simavr "$(elf_of "${variant}" "${EXAMPLE}")")"
    if diff -u --label expected --label "${variant}" \
        "${EXPECTED}" <(echo "${actual}"); then
      echo "${EXAMPLE} (${variant}): as expected"
    else
      failed=1
    fi
  done
  exit "${failed}"
}

main "$@"
