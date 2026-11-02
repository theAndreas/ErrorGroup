// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// What a sketch includes. arduino-cli finds a library only by a header that
// lies directly in its src/, not by <utl/error.h>; once this header has found
// it, src/ is on the include path and <utl/...> works in every file. The name
// is the library's, as Arduino expects, not lower case as Google's.

#ifndef ERRORGROUP_SRC_ERRORGROUP_H_
#define ERRORGROUP_SRC_ERRORGROUP_H_

#include "utl/error.h"
#include "utl/error_group.h"
#include "utl/result.h"

#endif  // ERRORGROUP_SRC_ERRORGROUP_H_
