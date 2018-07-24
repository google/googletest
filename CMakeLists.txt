cmake_minimum_required(VERSION 2.8.8)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(googletest-distribution)
set(GOOGLETEST_VERSION 1.9.0)

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

option(BUILD_GTEST "Builds the googletest subproject" OFF)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)

cmake_dependent_option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON "BUILD_GTEST OR BUILD_GMOCK" OFF)
cmake_dependent_option(INSTALL_GMOCK "Enable installation of googlemock. (Projects embedding googlemock may want to turn this OFF.)" ON "BUILD_GMOCK" OFF)

if(WIN32)
  set(INSTALL_CMAKE_DIR "cmake" CACHE PATH "CMake exported targets")
else()
  set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/gtest" CACHE PATH "CMake exported targets")
endif()

set(googletest_install_targets)
if(BUILD_GMOCK)
  add_subdirectory( googlemock )
elseif(BUILD_GTEST)
  add_subdirectory( googletest )
endif()

if(googletest_install_targets)
  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/googletestConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/googletestConfig.cmake"
    INSTALL_DESTINATION "${INSTALL_CMAKE_DIR}")
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/googletestConfigVersion.cmake
    VERSION "${GOOGLETEST_VERSION}"
    COMPATIBILITY SameMajorVersion)
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/googletestConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/googletestConfigVersion.cmake
    DESTINATION "${INSTALL_CMAKE_DIR}")
endif()
