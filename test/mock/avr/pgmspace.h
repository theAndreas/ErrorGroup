// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// avr-libc's access to flash, for the tests on the host, which has no
// separate flash: PROGMEM is nothing, and a read from flash is a read.

#ifndef UTL_TEST_MOCK_AVR_PGMSPACE_H_
#define UTL_TEST_MOCK_AVR_PGMSPACE_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define PROGMEM
#define PGM_P const char*

inline uint8_t pgm_read_byte(void const* address) {
  return *static_cast<uint8_t const*>(address);
}

inline void const* pgm_read_ptr(void const* address) {
  return *static_cast<void const* const*>(address);
}

inline int memcmp_P(void const* ram, void const* flash, size_t count) {
  return memcmp(ram, flash, count);
}

inline size_t strlen_P(char const* flash) { return strlen(flash); }

#endif  // UTL_TEST_MOCK_AVR_PGMSPACE_H_
