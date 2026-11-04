#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Checks the style of the C++ sources (clang-format, cpplint, and the indent
# of the lines that continue a Doxygen command, tools/doc_indent.pl), of the
# scripts (shellcheck) and of the CI workflow (actionlint). With --fix, formats
# the C++ sources instead of only checking them. The tools come from PyPI
# through uvx, at fixed versions, so that nothing has to be installed and
# every machine checks the same; doc_indent.pl needs only Perl.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly CLANG_FORMAT="clang-format==23.1.3"
readonly CPPLINT="cpplint==2.0.2"
readonly SHELLCHECK="shellcheck-py==0.11.0.1"
readonly ACTIONLINT="actionlint-py==1.7.12.25"

main() {
  local fix=0
  if [[ "${1:-}" == "--fix" ]]; then
    fix=1
  elif [[ $# -gt 0 ]]; then
    err "Usage: $0 [--fix]"
    exit 2
  fi

  cd "${ROOT_DIR}"
  # Tracked files and new ones not yet added, but not what .gitignore names:
  # a file is checked before its first commit, not after.
  local files=(git ls-files --cached --others --exclude-standard)
  local sources
  mapfile -t sources < <("${files[@]}" 'src/*' 'examples/*' 'test/*' \
    | grep -E '\.(h|cpp|ino)$')
  local scripts
  mapfile -t scripts < <("${files[@]}" 'tools/*.sh')

  local failed=0
  # The lines that continue a command of a Doxygen block start under its text,
  # docs/Documentation.md. clang-format keeps them so, but a line it breaks
  # starts at the margin again, so the indent comes after it.
  if ((fix)); then
    uvx --from "${CLANG_FORMAT}" clang-format -i "${sources[@]}"
    tools/doc_indent.pl --fix "${sources[@]}"
  else
    uvx --from "${CLANG_FORMAT}" clang-format --dry-run --Werror \
      "${sources[@]}" || failed=1
    tools/doc_indent.pl --check "${sources[@]}" || failed=1
  fi
  uvx "${CPPLINT}" --quiet "${sources[@]}" || failed=1
  uvx --from "${SHELLCHECK}" shellcheck -x "${scripts[@]}" || failed=1
  uvx --from "${ACTIONLINT}" actionlint || failed=1
  exit "${failed}"
}

main "$@"
