#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Builds the reference from the Doxygen blocks into build/docs/html and fails
# on a warning: a block Doxygen cannot read, a \param that names no
# parameter, an entity of the API without a block. Two kinds of "is not
# documented" are let pass and counted: in a header not yet in CONVERTED,
# whose comments are still the plain // of before the style of
# docs/Documentation.md, and in utl::internal, which a sketch does not use;
# but not in internal/fluent.h, whose functions Status and Result give a
# sketch. A missing \return is let pass too: a one-line block whose \brief
# says what comes back, "Returns the number of elements.", needs none, while
# every parameter needs its \param. Doxygen cannot be told to fail on some warnings and not others, so
# they go into a log and are sorted here. Doxygen comes from apt, in the
# Dockerfile and in CI alike, as it is not on PyPI.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly DOCS_DIR="${BUILD_DIR}/docs"
readonly LOG="${DOCS_DIR}/warnings.log"

# The headers whose comments are in the style of docs/Documentation.md, where
# a gap fails. A header joins with the pull request that converts it; once
# every header has, the list goes, and every gap fails everywhere.
readonly CONVERTED=(
  src/Utl.h
  src/utl/charconv_error.h
  src/utl/config.h
  src/utl/error.h
  src/utl/error_group.h
  src/utl/index_error.h
  src/utl/internal/fluent.h
  src/utl/messages.h
  src/utl/misuse.h
  src/utl/optional.h
  src/utl/optional_error.h
  src/utl/reference_wrapper.h
  src/utl/result.h
  src/utl/span.h
  src/utl/status.h
)

# Prints the warnings that fail, one per line, with paths from the root.
failing_warnings() {
  local converted
  converted="$(printf '%s\n' "${CONVERTED[@]}" | sed 's/\./\\./g' | paste -sd '|' -)"
  sed "s#^${ROOT_DIR}/##" "${LOG}" \
    | grep -v '^$' \
    | awk -v converted="^(${converted}):" '
        /is not documented\.$/ && $0 !~ converted { next }
        /is not documented\.$/ && /utl::internal/ \
            && !/^src\/utl\/internal\/fluent\.h:/ { next }
        /warning: return type of member .* is not documented$/ { next }
        { print }'
}

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi
  if ! command -v doxygen >/dev/null; then
    err "doxygen is missing; it comes from apt, see the Dockerfile."
    exit 1
  fi

  cd "${ROOT_DIR}"
  mkdir -p "${DOCS_DIR}"
  doxygen Doxyfile

  local all
  all="$(grep -c 'warning:' "${LOG}" || true)"
  local failing
  failing="$(failing_warnings)"
  if [[ -n "${failing}" ]]; then
    echo "${failing}"
  fi
  local count=0
  if [[ -n "${failing}" ]]; then
    count="$(wc -l <<< "${failing}")"
  fi
  echo "docs.sh: ${count} warnings that fail, $((all - count)) gaps let" \
    "pass: in headers not yet converted, in utl::internal, or a missing" \
    "return;" \
    "the reference is in ${DOCS_DIR}/html."
  if ((count > 0)); then
    exit 1
  fi
}

main "$@"
