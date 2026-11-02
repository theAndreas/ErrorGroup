// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// A Print that keeps what is printed, for the tests of PrintTo().

#ifndef UTL_TEST_STRING_PRINT_H_
#define UTL_TEST_STRING_PRINT_H_

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include <string>

namespace utl_test {

class StringPrint : public Print {
 public:
  size_t write(uint8_t c) override {
    text.push_back(static_cast<char>(c));
    return 1;
  }

  std::string text;
};

}  // namespace utl_test

#endif  // UTL_TEST_STRING_PRINT_H_
