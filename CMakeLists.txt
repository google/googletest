# Note: CMake support is community-based. The maintainers do not use CMake
# internally.

cmake_minimum_required(VERSION 2.8.8)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(googletest-distribution)
set(GOOGLETEST_VERSION 1.10.0)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.1")
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED OFF)
endif()

if(WIN32)
  if(MSVC)
    if(MSVC_TOOLSET_VERSION EQUAL 140 OR MSVC_TOOLSET_VERSION EQUAL 140)
      # there is a bug reported on https://developercommunity.visualstudio.com/content/problem/9696/c17-not-available-by-default.html
      # string_view is not available unless we use the /std:c++latest switch
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
    endif()
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
