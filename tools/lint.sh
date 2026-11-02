#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Checks the style of the C++ sources (clang-format, cpplint) and of the
# scripts (shellcheck). With --fix, formats the C++ sources instead of only
# checking them. The tools come from PyPI through uvx, at fixed versions, so
# that nothing has to be installed and every machine checks the same.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly CLANG_FORMAT="clang-format==23.1.3"
readonly CPPLINT="cpplint==2.0.2"
readonly SHELLCHECK="shellcheck-py==0.11.0.1"

main() {
  local fix=0
  if [[ "${1:-}" == "--fix" ]]; then
    fix=1
  elif [[ $# -gt 0 ]]; then
    err "Usage: $0 [--fix]"
    exit 2
  fi

  cd "${ROOT_DIR}"
  local sources
  mapfile -t sources < <(git ls-files 'src/*' 'examples/*' \
    | grep -E '\.(h|cpp|ino)$')
  local scripts
  mapfile -t scripts < <(git ls-files 'tools/*.sh')

  local failed=0
  if ((fix)); then
    uvx --from "${CLANG_FORMAT}" clang-format -i "${sources[@]}"
  else
    uvx --from "${CLANG_FORMAT}" clang-format --dry-run --Werror \
      "${sources[@]}" || failed=1
  fi
  uvx "${CPPLINT}" --quiet "${sources[@]}" || failed=1
  uvx --from "${SHELLCHECK}" shellcheck -x "${scripts[@]}" || failed=1
  exit "${failed}"
}

main "$@"
