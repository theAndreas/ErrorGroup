#!/bin/bash
#
# Compiles every example in both variants and prints flash and RAM of each,
# as a Markdown table. Fails on a warning outside the Arduino core, and if a
# vtable of the flash variant lies in RAM: then some file was compiled without
# the plugin, and a virtual call through it would jump anywhere.

set -euo pipefail

source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# Prints the number after "<label> use(s)" in the output of arduino-cli.
size_of() {
  local label="$1"
  local output="$2"
  sed -n "s/^${label} uses* \([0-9]*\) bytes.*/\1/p" <<< "${output}"
}

# Fails if a vtable lies in .data, that is, in RAM.
check_vtables_in_flash() {
  local elf="$1"
  local in_ram
  in_ram="$("$(avr_gcc_bin_dir)/avr-nm" -C "${elf}" \
    | grep ' [dD] vtable for ' || true)"
  if [[ -n "${in_ram}" ]]; then
    err "vtables in RAM despite the plugin, in ${elf}:"
    err "${in_ram}"
    return 1
  fi
}

main() {
  local failed=0
  local example
  local variant
  local output
  local warnings
  local -A flash_size
  local -A ram_size

  for example in $(ls "${ROOT_DIR}/examples"); do
    for variant in "${VARIANTS[@]}"; do
      if ! output="$(compile_example "${variant}" "${example}")"; then
        err "${example} (${variant}) does not compile:"
        err "${output}"
        failed=1
        continue
      fi
      warnings="$(grep 'warning:' <<< "${output}" \
        | grep -v '/cores/arduino/' || true)"
      if [[ -n "${warnings}" ]]; then
        err "${example} (${variant}) warns:"
        err "${warnings}"
        failed=1
      fi
      if [[ "${variant}" == "flash" ]]; then
        check_vtables_in_flash "$(elf_of flash "${example}")" || failed=1
      fi
      flash_size["${example}/${variant}"]="$(size_of 'Sketch' "${output}")"
      ram_size["${example}/${variant}"]="$(size_of 'Global variables' \
        "${output}")"
    done
  done

  echo "| Example | Flash, vtables in RAM | Flash, in flash |" \
    "RAM, vtables in RAM | RAM, in flash |"
  echo "|---|---|---|---|---|"
  for example in $(ls "${ROOT_DIR}/examples"); do
    echo "| ${example} | ${flash_size["${example}/ram"]:-} |" \
      "${flash_size["${example}/flash"]:-} |" \
      "${ram_size["${example}/ram"]:-} | ${ram_size["${example}/flash"]:-} |"
  done
  exit "${failed}"
}

main "$@"
