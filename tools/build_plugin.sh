#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Builds the GCC plugin avr-flash-vtbl into build/, against the plugin headers
# of the avr-gcc that arduino-cli uses. A plugin only loads into the cc1plus
# it was built for, so build it again after the AVR core is updated.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

main() {
  local bin_dir
  bin_dir="$(avr_gcc_bin_dir)"
  if [[ -z "${bin_dir}" ]]; then
    err "arduino-cli knows no avr-gcc for ${FQBN}; install arduino:avr."
    exit 1
  fi

  local plugin_dir
  plugin_dir="$("${bin_dir}/avr-g++" -print-file-name=plugin)"
  if [[ ! -d "${plugin_dir}/include" ]]; then
    err "${plugin_dir}/include is missing: this avr-gcc has no plugin headers."
    exit 1
  fi

  mkdir -p "${BUILD_DIR}"
  g++ -shared -fPIC -fno-rtti -O2 \
    -I"${plugin_dir}/include" \
    "${ROOT_DIR}/tools/avr-flash-vtbl/avr-flash-vtbl.c" \
    -o "${PLUGIN}"
  echo "Built ${PLUGIN}"
}

main "$@"
