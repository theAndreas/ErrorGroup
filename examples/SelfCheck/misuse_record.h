// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// SelfCheck's own report of a misuse, with UTL_CHECK_ACCESS at 1: it counts
// rather than prints, so that SelfCheck can check what was reported.

#ifndef UTL_EXAMPLES_SELFCHECK_MISUSE_RECORD_H_
#define UTL_EXAMPLES_SELFCHECK_MISUSE_RECORD_H_

#include <stdint.h>
#include <utl/misuse.h>

namespace self_check {

// How many misuses were reported, and the last.
uint8_t MisuseCount();
utl::Misuse LastMisuse();

}  // namespace self_check

#endif  // UTL_EXAMPLES_SELFCHECK_MISUSE_RECORD_H_
