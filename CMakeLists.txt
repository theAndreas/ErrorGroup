# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# The tests on the host: the library against a mock of the Arduino core, on
# GoogleTest, in three variants as SelfCheck runs in simavr. The library is
# built as gnu++11, as avr-gcc builds it; only the tests are C++17, which
# GoogleTest needs.
#
#   cmake --workflow --preset default    # the tests
#   cmake --workflow --preset sanitize   # under ASan and UBSan
#   tools/coverage.sh                    # the tests, and what they reach

cmake_minimum_required(VERSION 3.25)
project(Utl LANGUAGES CXX)
enable_testing()
# For clangd, which reads it through .clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(UTL_COVERAGE "Build with --coverage, for gcovr" OFF)
option(UTL_SANITIZE "Build with AddressSanitizer and UBSan" OFF)

find_package(GTest REQUIRED)
include(GoogleTest)

set(warnings -Wall -Wextra -Werror)
set(instrument)
if(UTL_COVERAGE)
  list(APPEND instrument --coverage)
endif()
if(UTL_SANITIZE)
  list(APPEND instrument -fsanitize=address,undefined
       -fno-sanitize-recover=all -fno-omit-frame-pointer)
endif()

file(GLOB library_sources CONFIGURE_DEPENDS src/utl/*.cpp)
set(test_sources
    test/array_test.cc
    test/error_test.cc
    test/flash_string_view_test.cc
    test/misuse_test.cc
    test/optional_test.cc
    test/result_test.cc
    test/span_test.cc
    test/status_test.cc
    test/string_view_test.cc
    test/alpha_error.cc
    test/beta_error.cc)

# One library and one test program per variant: the switches change what is
# compiled, so each variant is a build of its own.
function(add_variant name)
  set(defines ${ARGN})

  add_library(utl_${name} STATIC ${library_sources} test/mock/Arduino.cpp)
  target_include_directories(utl_${name} PUBLIC src test/mock)
  target_compile_definitions(utl_${name} PUBLIC ${defines})
  target_compile_options(utl_${name} PRIVATE -std=gnu++11 ${warnings}
                                              ${instrument})
  target_link_options(utl_${name} PUBLIC ${instrument})

  add_executable(utl_test_${name} ${test_sources})
  target_compile_features(utl_test_${name} PRIVATE cxx_std_17)
  # The root, so that a test includes its helpers as "test/...", as Google
  # wants an include to name the directory.
  target_include_directories(utl_test_${name} PRIVATE ${CMAKE_SOURCE_DIR})
  target_compile_options(utl_test_${name} PRIVATE ${warnings} ${instrument})
  target_link_libraries(utl_test_${name} PRIVATE utl_${name} GTest::gtest_main)
  gtest_discover_tests(utl_test_${name} TEST_PREFIX ${name}/)
endfunction()

add_variant(default)
add_variant(notexts UTL_ERROR_TEXTS=0)
add_variant(checked UTL_CHECK_ACCESS=1)
