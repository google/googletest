cmake_minimum_required(VERSION 2.6.2)

project( googletest-distribution )

enable_testing()

option(BUILD_GTEST "Builds the googletest subproject" OFF)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)

# this should preferably be ON
option(BUILD_WITH_DEBUG_POSTFIX "Builds libraries with a debug postfix 'd'" OFF)

if(BUILD_WITH_DEBUG_POSTFIX)
	set(CMAKE_DEBUG_POSTFIX "d")
endif()

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
elseif(BUILD_GTEST)
  add_subdirectory( googletest )
endif()
