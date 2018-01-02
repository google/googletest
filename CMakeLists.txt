cmake_minimum_required(VERSION 2.6.4)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project( googletest-distribution )

enable_testing()

include(CMakeDependentOption)
if (CMAKE_VERSION VERSION_LESS 2.8.5)
  set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "User executables (bin)")
  set(CMAKE_INSTALL_LIBDIR "lib${LIB_SUFFIX}" CACHE STRING "Object code libraries (lib)")
  set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "C header files (include)")
  mark_as_advanced(CMAKE_INSTALL_BINDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR)
else()
  include(GNUInstallDirs)
endif()

option(BUILD_GTEST "Builds the googletest subproject" OFF)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)

cmake_dependent_option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON "BUILD_GTEST OR BUILD_GMOCK" OFF)
cmake_dependent_option(INSTALL_GMOCK "Enable installation of googlemock. (Projects embedding googlemock may want to turn this OFF.)" ON "BUILD_GMOCK" OFF)

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
elseif(BUILD_GTEST)
  add_subdirectory( googletest )
endif()
