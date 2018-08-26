cmake_minimum_required(VERSION 2.8.8)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(googletest-distribution)
set(GOOGLETEST_VERSION 1.9.0)

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

# Define helper functions and macros
include(cmake/internal_utils.cmake)

# Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON)

CMAKE_DEPENDENT_OPTION(
  INSTALL_GTEST_CMAKE "Enable installation of googletest cmake config files." ON
  "INSTALL_GTEST" OFF
)
set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake" CACHE PATH "CMake exported targets")

CMAKE_DEPENDENT_OPTION(
  INSTALL_GTEST_PKGCONFIG "Enable installation of googletest pkg-config files." ON
  "INSTALL_GTEST" OFF
)
set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE PATH "pkg-config files")

set(GTEST_PACKAGE_NAME GTest CACHE STRING "googletest package name")
mark_as_advanced(GTEST_PACKAGE_NAME)
string(TOUPPER "${GTEST_PACKAGE_NAME}" GTEST_PACKAGE_NAME_UPPER)

if(BUILD_GMOCK)
  add_subdirectory(googlemock)
else()
  add_subdirectory(googletest)
endif()

# Install main cmake config file, version file, and etc.
install_project()
