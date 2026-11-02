// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The default of ReportMisuse(), which prints on Serial, in a file of its
// own, as in MaxMatrix: a sketch that defines its own report would otherwise
// pay for Serial all the same, 175 bytes of RAM on the Uno. The reference to
// Serial here pulls HardwareSerial0.o out of the core even when the default
// is left out, and serialEventRun() in main() keeps Serial alive from then on.
// So the library is linked as an archive (dot_a_linkage in
// library.properties), and this file is only taken when nothing else defines
// the report.

#include "utl/config.h"  // IWYU pragma: keep

#if UTL_CHECK_ACCESS

#include "utl/misuse.h"

namespace utl {

[[gnu::weak]] void ReportMisuse(Misuse misuse) {
  PrintMisuse(Serial, misuse);
  Serial.println();
}

}  // namespace utl

#endif
