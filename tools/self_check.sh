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
# SelfCheck ends within a second; one that runs longer has hung, or crashed
# into a loop of resets, as a call through a null pointer jumps to address 0.
readonly TIMEOUT_SECONDS=10

# Prints what the sketch sent on the UART, and returns the exit status of
# simavr: 124 if it did not end in time. simavr writes the UART to stderr, a
# line in color with CR LF shown as "..", and its own messages to stdout.
run_in_simavr() {
  local elf="$1"
  local uart
  local status=0
  uart="$(timeout "${TIMEOUT_SECONDS}" simavr -m atmega328p -f 16000000 \
    "${elf}" 2>&1 >/dev/null)" || status=$?
  sed -e 's/\x1b\[[0-9;]*m//g' -e 's/\.\.$//' <<< "${uart}" \
    | grep -v '^$' || true
  return "${status}"
}

main() {
  local failed=0
  local variant
  local output
  local actual
  local status

  for variant in "${VARIANTS[@]}"; do
    if ! output="$(compile_example "${variant}" "${EXAMPLE}")"; then
      err "${EXAMPLE} (${variant}) does not compile:"
      err "${output}"
      failed=1
      continue
    fi
    status=0
    actual="$(run_in_simavr "$(elf_of "${variant}" "${EXAMPLE}")")" || status=$?
    if ((status == 124)); then
      err "${EXAMPLE} (${variant}) did not end within ${TIMEOUT_SECONDS} s:" \
        "it hung or crashed. What it printed until then:"
      failed=1
    elif ((status != 0)); then
      err "${EXAMPLE} (${variant}): simavr ended with ${status}."
      failed=1
    fi
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
