cmake_minimum_required(VERSION 3.16)
project(googletest-distribution)
set(GOOGLETEST_VERSION 1.16.0)

if(CONFIG_GTEST)
    zephyr_library()

    zephyr_include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/include
        ${CMAKE_CURRENT_SOURCE_DIR}/googlemock/include
    )

    zephyr_library_sources(
        googletest/src/gtest-all.cc
        googlemock/src/gmock-all.cc
    )
endif()
