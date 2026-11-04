// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// array and span.

#include <Utl.h>
#include <stddef.h>
#include <stdint.h>

#include "self_check.h"

namespace self_check {

using utl::IndexError;
using utl::Result;
using utl::span;

namespace {

void CheckArray() {
  utl::array<uint8_t, 3> rows = {{codes[0], codes[1], codes[2]}};

  Check(rows.size() == 3 && !rows.empty(), F("array::size"));
  Check(rows[2] == 7 && rows.front() == 0 && rows.back() == 7,
        F("array::operator[], front, back"));
  uint16_t sum = 0;
  for (uint8_t const row : rows) {
    sum += row;
  }
  Check(sum == 8, F("array in a range-based for"));
  Check(rows.at(2).ValueOr(0) == 7, F("array::at"));
  Result<uint8_t> const beyond = rows.at(3);
  PrintStatus(beyond.status());
  Check(beyond.status() == IndexError::kOutOfRange,
        F("array::at beyond the end"));
  Check(rows.data() == &rows.front(), F("array::data"));

  utl::array<uint8_t, 3> copy = rows;
  Check(copy == rows, F("array copies and compares"));
  copy.fill(5);
  Check(copy != rows && copy[0] == 5 && copy[2] == 5, F("array::fill"));
}

// Takes a C array, a utl::array and a span alike, through a span of const.
uint16_t Sum(span<uint8_t const> values) {
  uint16_t sum = 0;
  for (uint8_t const value : values) {
    sum += value;
  }
  return sum;
}

void CheckSpan() {
  utl::array<uint8_t, 4> rows = {{codes[0], codes[1], codes[2], 3}};
  utl::array<uint8_t, 4> const& const_rows = rows;
  // The C array is what this checks a span of.
  uint8_t c_array[] = {codes[2], codes[2]};  // NOLINT(modernize-avoid-c-arrays)
  span<uint8_t> const all = rows;

  Check(span<uint8_t>().empty() && span<uint8_t>().data() == nullptr,
        F("span() is empty"));
  Check(all.size() == 4 && all.data() == rows.data() && all.size_bytes() == 4,
        F("span of an array"));
  Check(Sum(rows) == 11 && Sum(const_rows) == 11 && Sum(c_array) == 14 &&
            Sum(all) == 11,
        F("span of const from array, const array, C array, span"));
  all[0] = 9;
  Check(rows[0] == 9 && all.front() == 9 && all.back() == 3,
        F("span writes through"));
  all[0] = 0;

  Check(all.at(3).ValueOr(0) == 3 &&
            all.at(4).status() == IndexError::kOutOfRange,
        F("span::at"));
  Check(Sum(all.first(2).value()) == 1 && Sum(all.last(1).value()) == 3,
        F("span::first, last"));
  Check(Sum(all.subspan(1, 2).value()) == 8 && all.subspan(4).value().empty(),
        F("span::subspan"));
  Check(all.first(5).status() == IndexError::kOutOfRange &&
            all.last(5).status() == IndexError::kOutOfRange &&
            all.subspan(5).status() == IndexError::kOutOfRange &&
            all.subspan(2, 3).status() == IndexError::kOutOfRange,
        F("span beyond the end"));
  Check(all.subspan(1, static_cast<size_t>(-1)).status() ==
            IndexError::kOutOfRange,
        F("span::subspan does not overflow"));
  Check(all.subspan(1)
                .AndThen([](span<uint8_t> rest) { return rest.at(1); })
                .ValueOr(0) == 7,
        F("span with the fluent API"));
}

}  // namespace

void CheckContainers() {
  CheckArray();
  CheckSpan();
}

}  // namespace self_check
