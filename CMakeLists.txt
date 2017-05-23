cmake_minimum_required(VERSION 2.6.4)

project( googletest-distribution )

enable_testing()

option(BUILD_GTEST "Builds the googletest subproject" OFF)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)

if( ANDROID_STUDIO_WRAPPED_EXE )
    add_definitions( -DGTEST_REDIRECT_TO_ANDROID_LOG )
endif()

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
elseif(BUILD_GTEST)
  add_subdirectory( googletest )
endif()
