cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED GOOGLETEST_SOURCE_DIR)
  message(FATAL_ERROR "GOOGLETEST_SOURCE_DIR must be set")
endif()

if(NOT DEFINED GOOGLETEST_BINARY_DIR)
  message(FATAL_ERROR "GOOGLETEST_BINARY_DIR must be set")
endif()

function(write_output_directory_test_project source_dir)
  file(MAKE_DIRECTORY "${source_dir}")
  file(WRITE "${source_dir}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.16)
project(googletest_output_directory_test LANGUAGES CXX C)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

add_subdirectory("${GOOGLETEST_SOURCE_DIR}" googletest)

function(expect_target_property target property expected)
  get_target_property(actual "${target}" "${property}")
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR
      "${target} ${property}: expected '${expected}', got '${actual}'")
  endif()
endfunction()

function(expect_output_directories target)
  expect_target_property(
    "${target}" RUNTIME_OUTPUT_DIRECTORY
    "${EXPECTED_RUNTIME_OUTPUT_DIRECTORY}")
  expect_target_property(
    "${target}" LIBRARY_OUTPUT_DIRECTORY
    "${EXPECTED_LIBRARY_OUTPUT_DIRECTORY}")
  expect_target_property(
    "${target}" ARCHIVE_OUTPUT_DIRECTORY
    "${EXPECTED_ARCHIVE_OUTPUT_DIRECTORY}")
  expect_target_property(
    "${target}" PDB_OUTPUT_DIRECTORY
    "${EXPECTED_PDB_OUTPUT_DIRECTORY}")
  expect_target_property(
    "${target}" COMPILE_PDB_OUTPUT_DIRECTORY
    "${EXPECTED_COMPILE_PDB_OUTPUT_DIRECTORY}")
endfunction()

expect_output_directories(gtest)
expect_output_directories(gtest_main)
]=])
endfunction()

function(run_cmake_output_directory_test name)
  set(source_dir "${GOOGLETEST_BINARY_DIR}/${name}/src")
  set(binary_dir "${GOOGLETEST_BINARY_DIR}/${name}/build")
  file(REMOVE_RECURSE "${source_dir}" "${binary_dir}")
  write_output_directory_test_project("${source_dir}")

  set(cmake_args
    -S "${source_dir}"
    -B "${binary_dir}"
    "-DGOOGLETEST_SOURCE_DIR=${GOOGLETEST_SOURCE_DIR}")

  if(DEFINED GOOGLETEST_VERSION)
    list(APPEND cmake_args "-DGOOGLETEST_VERSION=${GOOGLETEST_VERSION}")
  endif()

  foreach(arg IN LISTS ARGN)
    list(APPEND cmake_args "${arg}")
  endforeach()

  execute_process(
    COMMAND "${CMAKE_COMMAND}" ${cmake_args}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
  if(result)
    message(FATAL_ERROR
      "Configuring ${name} failed:\n${output}\n${error}")
  endif()
endfunction()

set(default_binary_dir "${GOOGLETEST_BINARY_DIR}/default/build")
run_cmake_output_directory_test(default
  "-DEXPECTED_RUNTIME_OUTPUT_DIRECTORY=${default_binary_dir}/bin"
  "-DEXPECTED_LIBRARY_OUTPUT_DIRECTORY=${default_binary_dir}/lib"
  "-DEXPECTED_ARCHIVE_OUTPUT_DIRECTORY=${default_binary_dir}/lib"
  "-DEXPECTED_PDB_OUTPUT_DIRECTORY=${default_binary_dir}/bin"
  "-DEXPECTED_COMPILE_PDB_OUTPUT_DIRECTORY=${default_binary_dir}/lib")

set(custom_binary_dir "${GOOGLETEST_BINARY_DIR}/custom/build")
run_cmake_output_directory_test(custom
  "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/bin"
  "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/lib"
  "-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/archive"
  "-DCMAKE_PDB_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/pdb"
  "-DCMAKE_COMPILE_PDB_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/compile-pdb"
  "-DEXPECTED_RUNTIME_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/bin"
  "-DEXPECTED_LIBRARY_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/lib"
  "-DEXPECTED_ARCHIVE_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/archive"
  "-DEXPECTED_PDB_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/pdb"
  "-DEXPECTED_COMPILE_PDB_OUTPUT_DIRECTORY=${custom_binary_dir}/custom/compile-pdb")
