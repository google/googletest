########################################################################
# Note: CMake support is community-based. The maintainers do not use CMake
# internally.
#
# CMake build script for Google Mock.
#
# To run the tests for Google Mock itself on Linux, use 'make test' or
# ctest.  You can select which tests to run using 'ctest -R regex'.
# For more options, run 'ctest --help'.

if(NOT GOOGLETEST_TOPLEVEL)
    get_filename_component(gtest_toplevel ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
    message(FATAL_ERROR
        "Directly including GMock from ${CMAKE_CURRENT_LIST_DIR} is no longer "
        "supported. Please include the toplevel ${gtest_toplevel} instead."
    )
endif()

option(gmock_build_tests "Build all of Google Mock's own tests" ${GOOGLETEST_STANDALONE})
mark_as_advanced(gmock_build_tests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../googletest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Google Mock libraries.
add_library(gmock src/gmock-all.cc)
target_include_directories(gmock PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(gmock PROPERTIES
    VERSION ${PROJECT_VERSION}
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(gmock PRIVATE "GMOCK_CREATE_SHARED_LIBRARY=1" "GMOCK_LINKED_AS_SHARED_LIBRARY=0")
    target_compile_definitions(gmock INTERFACE "GMOCK_CREATE_SHARED_LIBRARY=0" "GMOCK_LINKED_AS_SHARED_LIBRARY=1")
else()
    target_compile_definitions(gmock PUBLIC "GMOCK_CREATE_SHARED_LIBRARY=0" "GMOCK_LINKED_AS_SHARED_LIBRARY=0")
endif()
target_link_libraries(gmock PUBLIC gtest)
add_library(GTest::gmock ALIAS gmock)

add_library(gmock_main src/gmock_main.cc)
target_link_libraries(gmock_main PUBLIC gmock)
add_library(GTest::gmock_main ALIAS gmock_main)

if(INSTALL_GTEST)
    set(gmock_compile_definitions "")
    get_target_property(defines gmock INTERFACE_COMPILE_DEFINITIONS)
    foreach(def IN LISTS defines)
        set(gmock_compile_definitions "${gmock_compile_definitions} -D${def}")
    endforeach()
    configure_file(cmake/gmock.pc.in gmock.pc @ONLY)
    configure_file(cmake/gmock_main.pc.in gmock_main.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gmock.pc ${CMAKE_CURRENT_BINARY_DIR}/gmock_main.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
    install(TARGETS gmock gmock_main EXPORT GMockTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(DIRECTORY include/gmock DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(BUILD_TESTING AND gmock_build_tests)
    gtest_cxx_test(gmock-actions_test gmock_main)
    gtest_cxx_test(gmock-cardinalities_test gmock_main)
    gtest_cxx_test(gmock_ex_test gmock_main)
    gtest_cxx_test(gmock-function-mocker_test gmock_main)
    gtest_cxx_test(gmock-internal-utils_test gmock_main)
    gtest_cxx_test(gmock-matchers_test gmock_main)
    gtest_cxx_test(gmock-more-actions_test gmock_main)
    gtest_cxx_test(gmock-nice-strict_test gmock_main)
    gtest_cxx_test(gmock-port_test gmock_main)
    gtest_cxx_test(gmock-spec-builders_test gmock_main)
    gtest_cxx_test(gmock_link_test gmock_main test/gmock_link2_test.cc)
    gtest_cxx_test(gmock_test gmock_main)

    if(DEFINED GTEST_HAS_PTHREAD)
        gtest_cxx_test(gmock_stress_test gmock)
    endif()

    gtest_cxx_executable(gmock_leak_test_ gmock_main test/gmock_leak_test_.cc)
    gtest_py_test(gmock_leak_test)

    gtest_cxx_executable(gmock_output_test_ gmock test/gmock_output_test_.cc)
    gtest_py_test(gmock_output_test)
endif()
