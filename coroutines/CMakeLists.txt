########################################################################
# Note: cotest is being brought up using cmake initially. MSVC and
# hermetic builds probably won't work, for the time being.
#
# CMake build script for cotest's coroutines support library.
#
# To run the tests for coroutines on Linux, use 'make test' or
# ctest.  You can select which tests to run using 'ctest -R regex'.
# For more options, run 'ctest --help'.
option(coro_build_samples "Build coro's sample programs." OFF)
option(coro_build_tests "Build all of coro's own tests." OFF)
option(gtest_disable_pthreads "Disable uses of pthreads in gtest." OFF)

# A directory to find Google Test sources.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gtest/CMakeLists.txt")
  set(gtest_dir gtest)
else()
  set(gtest_dir ../googletest)
endif()

# A directory to find Google Mock sources.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gmock/CMakeLists.txt")
  set(gmock_dir gmock)
else()
  set(gmock_dir ../googlemock)
endif()

########################################################################
#
# Project-wide settings

# Name of the project.
#
# CMake files in this project can refer to the root source directory
# as ${coro_SOURCE_DIR} and to the root binary directory as
# ${coro_BINARY_DIR}.
# Language "C" is required for find_package(Threads).

# Project version:
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)
project(cotest VERSION ${COROUTINES_VERSION} LANGUAGES CXX C)

if (POLICY CMP0063) # Visibility
  cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

# Instructs CMake to process Google Mock's CMakeLists.txt and add its
# targets to the current scope.  We are placing Google Mock's binary
# directory in a subdirectory of our own as VC compilation may break
# if they are the same (the default).
add_subdirectory("${gmock_dir}" "${cotest_BINARY_DIR}/${gmock_dir}")

# Define helper functions and macros used by Google Test.
include(${gtest_dir}/cmake/internal_utils.cmake)

config_compiler_and_linker()  # Defined in internal_utils.cmake.

# Adds coroutines header directories to the search path.
set(coro_build_include_dirs
  "${cotest_SOURCE_DIR}/include"
  "${cotest_SOURCE_DIR}"
  "${gmock_SOURCE_DIR}/include"
  # This directory is needed to build directly from Google Mock sources.
  "${gmock_SOURCE_DIR}")
include_directories(${coro_build_include_dirs})

########################################################################
#
# Defines the coroutines library.  This is an internal library.

# coroutines library.  We build it using more strict warnings than what
# are used for other targets, to ensure that cotest can be compiled by a user
# aggressive about warnings.

# For combined sources
#cxx_library(cotest "${cxx_strict}" "${gmock_dir}/src/gmock-all.cc" src/cotest-all.cc)

# For cotest development, helps keep tabs on deps
cxx_library(cotest 
            "${cxx_strict}" 
            "${gmock_dir}/src/gmock-all.cc" 
			src/cotest.cc
			src/cotest-coro-thread.cc
			src/cotest-crf-core.cc
			src/cotest-crf-launch.cc
			src/cotest-crf-mock.cc
			src/cotest-crf-payloads.cc
			src/cotest-crf-synch.cc
			src/cotest-crf-test.cc
			src/cotest-integ-finder.cc
			src/cotest-integ-mock.cc)


# Attach header directory information
# to the targets for when we are part of a parent build (ie being pulled
# in via add_subdirectory() rather than being a standalone build).
string(REPLACE ";" "$<SEMICOLON>" dirs "${coro_build_include_dirs}")
target_include_directories(cotest SYSTEM INTERFACE
  "$<BUILD_INTERFACE:${dirs}>"
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

########################################################################
#
# coroutine library tests.
#
# The tests are not built by default.  To build them, set the
# gtest_build_tests option to ON.  You can do it by running ccmake
# or specifying the -Dgtest_build_tests=ON flag when running cmake.
if (coro_build_tests)
  # Allow use of gtest
  include_directories(PRIVATE "${gtest_dir}/include" "${gmock_dir}/include")
  link_libraries(gtest gtest_main gmock)

  # This must be set in the root directory for the tests to be run by
  # 'make test' or ctest.
  enable_testing()

  ############################################################
  # Corountines internal tests
  cxx_test(coro-test-thread cotest)
  cxx_test(exp-finder-test cotest)

  ############################################################
  # Test cases that drive complete cotest - phase 1
  cxx_test(cotest-action-macro cotest)
  cxx_test(cotest-action-functor cotest)
  cxx_test(cotest-action-poly cotest)
  cxx_test(cotest-ui cotest)
  cxx_test(cotest-cardinality cotest)
  cxx_test(cotest-ext-filter cotest)
  cxx_test(cotest-int-filter cotest)
  cxx_test(cotest-lambda cotest)
  cxx_test(cotest-mockfunction cotest)
  cxx_test(cotest-wild cotest)
  cxx_test(cotest-types cotest)
    
  ############################################################
  # Test cases that drive complete cotest - phase 2
  cxx_test(cotest-launch cotest)
  cxx_test(cotest-launch-mock cotest)
  cxx_test(cotest-all-in cotest)
  cxx_test(cotest-mutex cotest)
  cxx_test(cotest-launch-multi-coro cotest)
  cxx_test(cotest-launch-lifetime cotest)
  cxx_test(cotest-serverised cotest)
  
endif()

