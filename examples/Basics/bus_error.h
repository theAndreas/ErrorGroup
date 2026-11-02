// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of a bus, the second group of this example. Its codes overlap
// with those of the font, which is fine: an error compares group and code.

#ifndef ERRORGROUP_EXAMPLES_BASICS_BUS_ERROR_H_
#define ERRORGROUP_EXAMPLES_BASICS_BUS_ERROR_H_

#include <error_group.h>

namespace bus {

enum class BusError : uint8_t {
  kNack,
  kTimeout,
};

class BusErrorGroup : public error_group::ErrorGroup {
 public:
  constexpr BusErrorGroup() = default;

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

// Defined in bus_error.cpp, and nowhere else: its address is the group.
extern const BusErrorGroup kBusErrorGroup;

inline const error_group::ErrorGroup& GetErrorGroup(BusError) {
  return kBusErrorGroup;
}

}  // namespace bus

#endif  // ERRORGROUP_EXAMPLES_BASICS_BUS_ERROR_H_
