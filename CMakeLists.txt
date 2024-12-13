# Note: CMake support is community-based. The maintainers do not use CMake
# internally.

cmake_minimum_required(VERSION 3.13)

set(GOOGLETEST_VERSION 1.15.2)
project(googletest-distribution
  VERSION ${GOOGLETEST_VERSION}
  DESCRIPTION "Google's C++ test framework"
  HOMEPAGE_URL "https://google.github.io/googletest"
  LANGUAGES CXX
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(PROJECT_IS_TOP_LEVEL TRUE)
endif()

if(NOT CYGWIN AND NOT MSYS AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL QNX)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

# Note that googlemock target already builds googletest.
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ${PROJECT_IS_TOP_LEVEL})
option(GTEST_HAS_ABSL "Use Abseil and RE2. Requires Abseil and RE2 to be separately added to the build." OFF)

if(GTEST_HAS_ABSL)
  if(NOT TARGET absl::base)
    find_package(absl REQUIRED)
  endif()
  if(NOT TARGET re2::re2)
    find_package(re2 REQUIRED)
  endif()
endif()

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
else()
  add_subdirectory( googletest )
endif()

if(PROJECT_IS_TOP_LEVEL)
  set(CPACK_PACKAGE_NAME "GTest") # same as the name in `find_package`
  set(CPACK_PACKAGE_VENDOR "Google")
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  include(CPack)
endif()
