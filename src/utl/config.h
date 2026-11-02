// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The switches of the library. Each is set for the whole build, never in a
// sketch: the library's own files and a group's .cpp have to see the same
// value. The Arduino IDE passes no flags per sketch, so a switch goes into
// platform.local.txt beside the platform.txt of the core,
//
//   compiler.cpp.extra_flags=-DUTL_ERROR_TEXTS=0
//
// or, with arduino-cli, into
//
//   --build-property "compiler.cpp.extra_flags=-DUTL_ERROR_TEXTS=0"

#ifndef UTL_SRC_UTL_CONFIG_H_
#define UTL_SRC_UTL_CONFIG_H_

// 1, the default: an error says what went wrong, "bus: nack". 0: the texts of
// the groups are left out, and an error prints its code, "bus: 0". It saves
// 46 bytes of flash in Basics, with two groups and four texts, and 62 in
// SelfCheck, after the code that prints the number in their place. A group's
// code stays the same either way: with the switch at 0, LookUpMessage() does
// not read its table, so the table and its texts are unused and the linker
// leaves them out.
#ifndef UTL_ERROR_TEXTS
#define UTL_ERROR_TEXTS 1
#endif

#endif  // UTL_SRC_UTL_CONFIG_H_
