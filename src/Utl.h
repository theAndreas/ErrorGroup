// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// What a sketch includes. arduino-cli finds a library only by a header that
// lies directly in its src/, not by <utl/error.h>; once this header has found
// it, src/ is on the include path and <utl/...> works in every file. The name
// is the library's, as Arduino expects, not lower case as Google's.

#ifndef UTL_SRC_UTL_H_
#define UTL_SRC_UTL_H_

#include "utl/array.h"        // IWYU pragma: export
#include "utl/error.h"        // IWYU pragma: export
#include "utl/error_group.h"  // IWYU pragma: export
#include "utl/index_error.h"  // IWYU pragma: export
#include "utl/result.h"       // IWYU pragma: export
#include "utl/span.h"         // IWYU pragma: export
#include "utl/status.h"       // IWYU pragma: export

#endif  // UTL_SRC_UTL_H_
