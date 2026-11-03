// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The switches of the library. Each is set for the whole build, never in a
// sketch: the library's own files and a group's .cpp have to see the same
// value. The Arduino IDE passes no flags per sketch, so a switch goes into
// platform.local.txt beside the platform.txt of the core,
//
//   compiler.cpp.extra_flags=-DUTL_CHECK_ACCESS=1
//
// or, with arduino-cli, into
//
//   --build-property "compiler.cpp.extra_flags=-DUTL_CHECK_ACCESS=1"

#ifndef UTL_SRC_UTL_CONFIG_H_
#define UTL_SRC_UTL_CONFIG_H_

// 0, the default: nothing is checked, and nothing is compiled for it. 1: an
// access that the standard leaves undefined, or that gives a placeholder
// here, is reported to utl::ReportMisuse() before it happens: value() of a
// Result with an error, * and -> of an empty optional, [] beyond the end,
// front() or back() of an empty span or string_view, and remove_prefix() or
// remove_suffix() of more than there is. For finding a mistake
// while a sketch is written, as the DET of AUTOSAR Classic; see misuse.h.
#ifndef UTL_CHECK_ACCESS
#define UTL_CHECK_ACCESS 0
#endif

#endif  // UTL_SRC_UTL_CONFIG_H_
