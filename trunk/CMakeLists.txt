########################################################################
# Experimental CMake build script for Google Test.
#
# Consider this a prototype.  It will change drastically.  For now,
# this is only for people on the cutting edge.
#
# To run the tests for Google Test itself on Linux, use 'make test' or
# ctest.  You can select which tests to run using 'ctest -R regex'.
# For more options, run 'ctest --help'.

########################################################################
#
# Project-wide settings

# Name of the project.
#
# CMake files in this project can refer to the root source directory
# as ${gtest_SOURCE_DIR} and to the root binary directory as
# ${gtest_BINARY_DIR}.
# Language "C" is required for find_package(Threads).
project(gtest CXX C)
cmake_minimum_required(VERSION 2.6.4)

# Where gtest's .h files can be found.
include_directories(
  ${gtest_SOURCE_DIR}/include
  ${gtest_SOURCE_DIR})

# Where the gtest libraries can be found.
link_directories(
  ${gtest_BINARY_DIR}/src)

# Defines CMAKE_USE_PTHREADS_INIT and CMAKE_THREAD_LIBS_INIT.
find_package(Threads)

# Defines the compiler/linker flags used to build gtest.  You can
# tweak these definitions to suit your need.
if (MSVC)
  set(cxx_base "${CMAKE_CXX_FLAGS} -GS -W4 -WX -wd4275 -nologo -J
                -Zi -D_UNICODE -DUNICODE -DWIN32 -D_WIN32 -DSTRICT
                -DWIN32_LEAN_AND_MEAN")
  set(cxx_default "${cxx_base} -EHsc -D_HAS_EXCEPTIONS=1")
else()
  set(cxx_base "${CMAKE_CXX_FLAGS}")

  if (CMAKE_USE_PTHREADS_INIT)  # The pthreads library is available.
    set(cxx_base "${cxx_base} -DGTEST_HAS_PTHREAD=1")
  endif()

  set(cxx_default "${cxx_base} -fexceptions")
endif()

########################################################################
#
# Defines the gtest & gtest_main libraries.  User tests should link
# with one of them.

function(cxx_library name cxx_flags)
  # ARGN refers to additional arguments after 'cxx_flags'.
  add_library(${name} STATIC ${ARGN})
  set_target_properties(${name}
    PROPERTIES
    COMPILE_FLAGS "${cxx_flags}")
    if (CMAKE_USE_PTHREADS_INIT)
      target_link_libraries(${name} ${CMAKE_THREAD_LIBS_INIT})
    endif()
endfunction()

cxx_library(gtest "${cxx_default}" src/gtest-all.cc)
cxx_library(gtest_main "${cxx_default}" src/gtest_main.cc)
target_link_libraries(gtest_main gtest)

########################################################################
#
# Samples on how to link user tests with gtest or gtest_main.
#
# They are not built by default.  To build them, set the
# build_gtest_samples option to ON.  You can do it by running ccmake
# or specifying the -Dbuild_gtest_samples=ON flag when running cmake.

option(build_gtest_samples "Build gtest's sample programs." OFF)

# cxx_executable(name dir lib srcs...)
#
# creates a named target that depends on the given lib and is built
# from the given source files.  dir/name.cc is implicitly included in
# the source file list.
function(cxx_executable name dir lib)
  add_executable(${name}
    ${dir}/${name}.cc
    ${ARGN})
  set_target_properties(${name}
    PROPERTIES
    COMPILE_FLAGS "${cxx_default}")
  target_link_libraries(${name} ${lib})
endfunction()

if (build_gtest_samples)
  cxx_executable(sample1_unittest samples gtest_main samples/sample1.cc)
  cxx_executable(sample2_unittest samples gtest_main samples/sample2.cc)
  cxx_executable(sample3_unittest samples gtest_main)
  cxx_executable(sample4_unittest samples gtest_main samples/sample4.cc)
  cxx_executable(sample5_unittest samples gtest_main samples/sample1.cc)
  cxx_executable(sample6_unittest samples gtest_main)
  cxx_executable(sample7_unittest samples gtest_main)
  cxx_executable(sample8_unittest samples gtest_main)
  cxx_executable(sample9_unittest samples gtest)
  cxx_executable(sample10_unittest samples gtest)
endif()

########################################################################
#
# Google Test's own tests.
#
# You can skip this section if you aren't interested in testing
# Google Test itself.
#
# Most of the tests are not built by default.  To build them, set the
# build_all_gtest_tests option to ON.  You can do it by running ccmake
# or specifying the -Dbuild_all_gtest_tests=ON flag when running cmake.

option(build_all_gtest_tests "Build all of gtest's own tests." OFF)

# This must be set in the root directory for the tests to be run by
# 'make test' or ctest.
enable_testing()

# Sets PYTHONINTERP_FOUND and PYTHON_EXECUTABLE.
find_package(PythonInterp)

############################################################
# C++ tests built with standard compiler flags.

# cxx_test_with_flags(name cxx_flags libs srcs...)
#
# creates a named C++ test that depends on the given libs and is built
# from the given source files with the given compiler flags.
function(cxx_test_with_flags name cxx_flags libs)
  add_executable(${name} ${ARGN})
  set_target_properties(${name}
    PROPERTIES
    COMPILE_FLAGS "${cxx_flags}")
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach (lib "${libs}")
    target_link_libraries(${name} ${lib})
  endforeach()
  add_test(${name} ${name})
endfunction()

# cxx_test(name libs srcs...)
#
# creates a named test target that depends on the given libs and is
# built from the given source files.  Unlike cxx_test_with_flags,
# test/name.cc is already implicitly included in the source file list.
function(cxx_test name libs)
  cxx_test_with_flags("${name}" "${cxx_default}" "${libs}"
    "test/${name}.cc" ${ARGN})
endfunction()

cxx_test(gtest_unittest gtest_main)

if (build_all_gtest_tests)
  cxx_test(gtest-death-test_test gtest_main)
  cxx_test(gtest_environment_test gtest)
  cxx_test(gtest-filepath_test gtest_main)
  cxx_test(gtest-linked_ptr_test gtest_main)
  cxx_test(gtest-listener_test gtest_main)
  cxx_test(gtest_main_unittest gtest_main)
  cxx_test(gtest-message_test gtest_main)
  cxx_test(gtest_no_test_unittest gtest)
  cxx_test(gtest-options_test gtest_main)
  cxx_test(gtest-param-test_test gtest
    test/gtest-param-test2_test.cc)
  cxx_test(gtest-port_test gtest_main)
  cxx_test(gtest_pred_impl_unittest gtest_main)
  cxx_test(gtest_prod_test gtest_main
    test/production.cc)
  cxx_test(gtest_repeat_test gtest)
  cxx_test(gtest_sole_header_test gtest_main)
  cxx_test(gtest_stress_test gtest)
  cxx_test(gtest-test-part_test gtest_main)
  cxx_test(gtest_throw_on_failure_ex_test gtest)
  cxx_test(gtest-typed-test_test gtest_main
    test/gtest-typed-test2_test.cc)
  cxx_test(gtest-unittest-api_test gtest)
endif()

############################################################
# C++ tests built with non-standard compiler flags.

if (MSVC)
  set(cxx_no_exception "${cxx_base} -D_HAS_EXCEPTIONS=0")
  set(cxx_no_rtti "${cxx_default} -GR-")
else()
  set(cxx_no_exception "${cxx_base} -fno-exceptions")
  set(cxx_no_rtti "${cxx_default} -fno-rtti -DGTEST_HAS_RTTI=0")
endif()
set(cxx_use_own_tuple "${cxx_default} -DGTEST_USE_OWN_TR1_TUPLE=1")

if (build_all_gtest_tests)
  cxx_library(gtest_no_exception "${cxx_no_exception}"
    src/gtest-all.cc)
  cxx_library(gtest_main_no_rtti "${cxx_no_rtti}"
    src/gtest-all.cc src/gtest_main.cc)

  cxx_test_with_flags(gtest_no_rtti_unittest "${cxx_no_rtti}"
    gtest_main_no_rtti test/gtest_unittest.cc)

  if (NOT(MSVC AND (MSVC_VERSION EQUAL 1600)))
    # The C++ Standard specifies tuple_element<int, class>.
    # Yet MSVC 10's <utility> declares tuple_element<size_t, class>.
    # That declaration conflicts with our own standard-conforming
    # tuple implementation.  Therefore using our own tuple with
    # MSVC 10 doesn't compile.
    cxx_library(gtest_main_use_own_tuple "${cxx_use_own_tuple}"
      src/gtest-all.cc src/gtest_main.cc)

    cxx_test_with_flags(gtest-tuple_test "${cxx_use_own_tuple}"
      gtest_main_use_own_tuple test/gtest-tuple_test.cc)

    cxx_test_with_flags(gtest_use_own_tuple_test "${cxx_use_own_tuple}"
      gtest_main_use_own_tuple
      test/gtest-param-test_test.cc test/gtest-param-test2_test.cc)
  endif()

endif()

############################################################
# Python tests.

# py_test(name)
#
# creates a Python test with the given name whose main module is in
# test/name.py.  It does nothing if Python is not installed.
function(py_test name)
  if (PYTHONINTERP_FOUND)
    # ${gtest_BINARY_DIR} is known at configuration time, so we can
    # directly bind it from cmake. ${CTEST_CONFIGURATION_TYPE} is known
    # only at ctest runtime (by calling ctest -c <Configuration>), so
    # we have to escape $ to delay variable substitution here.
    add_test(${name}
      ${PYTHON_EXECUTABLE} ${gtest_SOURCE_DIR}/test/${name}.py
          --gtest_build_dir=${gtest_BINARY_DIR}/\${CTEST_CONFIGURATION_TYPE})
  endif()
endfunction()

if (build_all_gtest_tests)
  cxx_executable(gtest_break_on_failure_unittest_ test gtest)
  py_test(gtest_break_on_failure_unittest)

  cxx_executable(gtest_color_test_ test gtest)
  py_test(gtest_color_test)

  cxx_executable(gtest_env_var_test_ test gtest)
  py_test(gtest_env_var_test)

  cxx_executable(gtest_filter_unittest_ test gtest)
  py_test(gtest_filter_unittest)

  cxx_executable(gtest_help_test_ test gtest_main)
  py_test(gtest_help_test)

  cxx_executable(gtest_list_tests_unittest_ test gtest)
  py_test(gtest_list_tests_unittest)

  cxx_executable(gtest_output_test_ test gtest)
  py_test(gtest_output_test)

  cxx_executable(gtest_shuffle_test_ test gtest)
  py_test(gtest_shuffle_test)

  cxx_executable(gtest_throw_on_failure_test_ test gtest_no_exception)
  set_target_properties(gtest_throw_on_failure_test_
    PROPERTIES
    COMPILE_FLAGS "${cxx_no_exception}")
  py_test(gtest_throw_on_failure_test)

  cxx_executable(gtest_uninitialized_test_ test gtest)
  py_test(gtest_uninitialized_test)

  cxx_executable(gtest_xml_outfile1_test_ test gtest_main)
  cxx_executable(gtest_xml_outfile2_test_ test gtest_main)
  py_test(gtest_xml_outfiles_test)

  cxx_executable(gtest_xml_output_unittest_ test gtest)
  py_test(gtest_xml_output_unittest)
endif()
