# Note: CMake support is community-based. The maintainers do not use CMake
# internally.

cmake_minimum_required(VERSION 2.8.8)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(googletest-distribution)
set(GOOGLETEST_VERSION 1.10.0)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.1")
  if(NOT CYGWIN AND NOT MSYS)
    set(CMAKE_CXX_EXTENSIONS OFF)
  endif()
endif()

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON)

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
else()
  add_subdirectory( googletest )
endif()
