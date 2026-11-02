#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs the tests on the host with --coverage and reports which lines of src/
# they reach: the lines no test reaches on the terminal, and all of it as HTML
# in build/coverage/html/. The three variants count together, as each takes
# other branches of #if. gcovr comes from PyPI through uvx, at a fixed
# version, like the tools of lint.sh.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly GCOVR="gcovr==8.8"
readonly COVERAGE_DIR="${BUILD_DIR}/coverage"

main() {
  cd "${ROOT_DIR}"
  # Counts from an earlier run would add to this one.
  find "${COVERAGE_DIR}" -name '*.gcda' -delete 2>/dev/null || true
  cmake --workflow --preset coverage

  mkdir -p "${COVERAGE_DIR}/html"
  # separate: a function such as internal::Check() lies on another line in
  # each variant, under its own #if, and is two functions, not one to merge.
  # --merge-lines: gcov counts a template per instance, and each lambda is a
  # type of its own, so a line reached by one instance stood as missed by
  # another; merged, a line counts as reached if any instance reaches it.
  uvx "${GCOVR}" --root "${ROOT_DIR}" --filter 'src/' \
    --merge-mode-functions=separate \
    --merge-lines \
    --txt-metric line --txt "${COVERAGE_DIR}/coverage.txt" \
    --html-details "${COVERAGE_DIR}/html/index.html" \
    --print-summary "${COVERAGE_DIR}"
  echo
  # The files with lines missed, and which.
  grep -v ' 100%' "${COVERAGE_DIR}/coverage.txt" || true
}

main "$@"
