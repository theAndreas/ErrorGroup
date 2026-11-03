// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The errors of a bus, the second group of this example. Its codes overlap
// with those of the font, which is fine: an error compares group and code.

#ifndef UTL_EXAMPLES_BASICS_BUS_ERROR_H_
#define UTL_EXAMPLES_BASICS_BUS_ERROR_H_

#include <stdint.h>
#include <utl/error_group.h>

namespace bus {

enum class BusError : uint8_t {
  kNack,
  kTimeout,
};

class BusErrorGroup : public utl::ErrorGroup {
 public:
  constexpr BusErrorGroup() = default;

  __FlashStringHelper const* Name() const override;
  __FlashStringHelper const* Message(uint8_t code) const override;
};

// Defined in bus_error.cpp, and nowhere else: its address is the group.
extern BusErrorGroup const kBusErrorGroup;

inline utl::ErrorGroup const& GetErrorGroup(BusError) { return kBusErrorGroup; }

}  // namespace bus

#endif  // UTL_EXAMPLES_BASICS_BUS_ERROR_H_
