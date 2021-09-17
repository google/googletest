########################################################################
# Note: CMake support is community-based. The maintainers do not use CMake
# internally.
#
# CMake build script for Google Test.
#
# To run the tests for Google Test itself on Linux, use 'make test' or
# ctest.  You can select which tests to run using 'ctest -R regex'.
# For more options, run 'ctest --help'.

if(NOT GOOGLETEST_TOPLEVEL)
    get_filename_component(gtest_toplevel ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
    message(FATAL_ERROR
        "Directly including GTest from ${CMAKE_CURRENT_LIST_DIR} is no longer "
        "supported. Please include the toplevel ${gtest_toplevel} instead."
    )
endif()

option(gtest_build_tests "Build all of gtest's own tests" ${GOOGLETEST_STANDALONE})
option(gtest_build_samples "Build gtest's sample programs" OFF)
option(gtest_disable_pthreads "Disable uses of pthreads in gtest" OFF)
option(gtest_hide_internal_symbols "Build gtest with internal symbols hidden in shared libraries" OFF)

mark_as_advanced(
    gtest_build_tests
    gtest_build_samples
    gtest_disable_pthreads
    gtest_hide_internal_symbols
)

set(gtest_has_pthread FALSE)
if(NOT gtest_disable_pthreads)
    enable_language(C)  # required for Threads package
    find_package(Threads)
    if (CMAKE_USE_PTHREADS_INIT)
        set(gtest_has_pthread TRUE)
    endif()
endif()
# Propagate gtest_has_pthread to parent CMakelists.txt
set(gtest_has_pthread "${gtest_has_pthread}" PARENT_SCOPE)

if (gtest_hide_internal_symbols)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Google Test libraries.
add_library(gtest src/gtest-all.cc)
target_include_directories(gtest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(gtest PROPERTIES
    VERSION ${PROJECT_VERSION}
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(gtest PRIVATE "GTEST_CREATE_SHARED_LIBRARY=1" "GTEST_LINKED_AS_SHARED_LIBRARY=0")
    target_compile_definitions(gtest INTERFACE "GTEST_CREATE_SHARED_LIBRARY=0" "GTEST_LINKED_AS_SHARED_LIBRARY=1")
else()
    target_compile_definitions(gtest PUBLIC "GTEST_CREATE_SHARED_LIBRARY=0" "GTEST_LINKED_AS_SHARED_LIBRARY=0")
endif()
if(gtest_has_pthread)
    target_link_libraries(gtest PUBLIC Threads::Threads)
    target_compile_definitions(gtest PUBLIC "GTEST_HAS_PTHREAD=1")
else()
    target_compile_definitions(gtest PUBLIC "GTEST_HAS_PTHREAD=0")
endif()

add_library(GTest::gtest ALIAS gtest)

add_library(gtest_main src/gtest_main.cc)
target_link_libraries(gtest_main PUBLIC gtest)
set_target_properties(gtest PROPERTIES
    VERSION ${PROJECT_VERSION}
)
add_library(GTest::gtest_main ALIAS gtest_main)

if(INSTALL_GTEST)
    set(gtest_compile_definitions "")
    get_target_property(defines gtest INTERFACE_COMPILE_DEFINITIONS)
    foreach(def IN LISTS defines)
        set(gtest_compile_definitions "${gtest_compile_definitions} -D${def}")
    endforeach()
    configure_file(cmake/gtest.pc.in gtest.pc @ONLY)
    configure_file(cmake/gtest_main.pc.in gtest_main.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gtest.pc ${CMAKE_CURRENT_BINARY_DIR}/gtest_main.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
    install(TARGETS gtest gtest_main EXPORT GTestTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(DIRECTORY include/gtest DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(gtest_build_samples)
    gtest_cxx_executable(sample1_unittest  gtest_main samples/sample1_unittest.cc samples/sample1.cc)
    gtest_cxx_executable(sample2_unittest  gtest_main samples/sample2_unittest.cc samples/sample2.cc)
    gtest_cxx_executable(sample3_unittest  gtest_main samples/sample3_unittest.cc)
    gtest_cxx_executable(sample4_unittest  gtest_main samples/sample4_unittest.cc samples/sample4.cc)
    gtest_cxx_executable(sample5_unittest  gtest_main samples/sample5_unittest.cc samples/sample1.cc)
    gtest_cxx_executable(sample6_unittest  gtest_main samples/sample6_unittest.cc)
    gtest_cxx_executable(sample7_unittest  gtest_main samples/sample7_unittest.cc)
    gtest_cxx_executable(sample8_unittest  gtest_main samples/sample8_unittest.cc)
    gtest_cxx_executable(sample9_unittest  gtest      samples/sample9_unittest.cc)
    gtest_cxx_executable(sample10_unittest gtest      samples/sample10_unittest.cc)
endif()

if(BUILD_TESTING AND gtest_build_tests)
    gtest_cxx_test(googletest-death-test-test gtest_main)
    gtest_cxx_test(gtest_environment_test gtest)
    gtest_cxx_test(googletest-filepath-test gtest_main)
    gtest_cxx_test(googletest-listener-test gtest_main)
    gtest_cxx_test(gtest_main_unittest gtest_main)
    gtest_cxx_test(googletest-message-test gtest_main)
    gtest_cxx_test(gtest_no_test_unittest gtest)
    gtest_cxx_test(googletest-options-test gtest_main)
    gtest_cxx_test(googletest-param-test-test gtest
        test/googletest-param-test2-test.cc)
    gtest_cxx_test(googletest-port-test gtest_main)
    gtest_cxx_test(gtest_pred_impl_unittest gtest_main)
    gtest_cxx_test(gtest_premature_exit_test gtest
        test/gtest_premature_exit_test.cc)
    gtest_cxx_test(googletest-printers-test gtest_main)
    gtest_cxx_test(gtest_prod_test gtest_main
        test/production.cc)
    gtest_cxx_test(gtest_repeat_test gtest)
    gtest_cxx_test(gtest_sole_header_test gtest_main)
    gtest_cxx_test(gtest_stress_test gtest)
    gtest_cxx_test(googletest-test-part-test gtest_main)
    gtest_cxx_test(gtest_throw_on_failure_ex_test gtest)
    gtest_cxx_test(gtest-typed-test_test gtest_main
        test/gtest-typed-test2_test.cc)
    gtest_cxx_test(gtest_unittest gtest_main)
    gtest_cxx_test(gtest-unittest-api_test gtest)
    gtest_cxx_test(gtest_skip_in_environment_setup_test gtest_main)
    gtest_cxx_test(gtest_skip_test gtest_main)

    gtest_cxx_executable(gtest-death-test_ex_nocatch_test gtest test/googletest-death-test_ex_test.cc)
    target_compile_definitions(gtest-death-test_ex_nocatch_test PRIVATE "GTEST_ENABLE_CATCH_EXCEPTIONS_=0")
    add_test(NAME gtest-death-test_ex_nocatch_test COMMAND gtest-death-test_ex_nocatch_test)

    gtest_cxx_executable(gtest-death-test_ex_catch_test gtest test/googletest-death-test_ex_test.cc)
    target_compile_definitions(gtest-death-test_ex_catch_test PRIVATE "GTEST_ENABLE_CATCH_EXCEPTIONS_=1")
    add_test(NAME gtest-death-test_ex_catch_test COMMAND gtest-death-test_ex_catch_test)

    gtest_cxx_executable(googletest-break-on-failure-unittest_ gtest test/googletest-break-on-failure-unittest_.cc)
    gtest_py_test(googletest-break-on-failure-unittest)
  
    gtest_py_test(gtest_skip_check_output_test)
    gtest_py_test(gtest_skip_environment_check_output_test)

    gtest_cxx_executable(googletest-catch-exceptions-ex-test_ gtest_main test/googletest-catch-exceptions-test_.cc)
    gtest_cxx_executable(googletest-catch-exceptions-no-ex-test_ gtest_main test/googletest-catch-exceptions-test_.cc)
    target_compile_definitions(googletest-catch-exceptions-no-ex-test_ PRIVATE "GTEST_HAS_EXCEPTIONS=0")

    gtest_py_test(googletest-catch-exceptions-test)
 
    gtest_cxx_executable(googletest-color-test_ gtest test/googletest-color-test_.cc)
    gtest_py_test(googletest-color-test)
  
    gtest_cxx_executable(googletest-env-var-test_ gtest test/googletest-env-var-test_.cc)
    gtest_py_test(googletest-env-var-test)
  
    gtest_cxx_executable(googletest-filter-unittest_ gtest test/googletest-filter-unittest_.cc)
    gtest_py_test(googletest-filter-unittest)
  
    gtest_cxx_executable(gtest_help_test_ gtest_main test/gtest_help_test_.cc)
    gtest_py_test(gtest_help_test)
  
    gtest_cxx_executable(googletest-list-tests-unittest_ gtest test/googletest-list-tests-unittest_.cc)
    gtest_py_test(googletest-list-tests-unittest)
  
    gtest_cxx_executable(googletest-output-test_ gtest test/googletest-output-test_.cc)
    gtest_py_test(googletest-output-test --no_stacktrace_support)
  
    gtest_cxx_executable(googletest-shuffle-test_ gtest test/googletest-shuffle-test_.cc)
    gtest_py_test(googletest-shuffle-test)

    gtest_cxx_executable(googletest-uninitialized-test_ gtest test/googletest-uninitialized-test_.cc)
    gtest_py_test(googletest-uninitialized-test)
  
    gtest_cxx_executable(gtest_list_output_unittest_ gtest test/gtest_list_output_unittest_.cc)
    gtest_py_test(gtest_list_output_unittest)
  
    gtest_cxx_executable(gtest_xml_outfile1_test_ gtest_main test/gtest_xml_outfile1_test_.cc)
    gtest_cxx_executable(gtest_xml_outfile2_test_ gtest_main test/gtest_xml_outfile2_test_.cc)
    gtest_py_test(gtest_xml_outfiles_test)
    gtest_py_test(googletest-json-outfiles-test)
  
    gtest_cxx_executable(gtest_xml_output_unittest_ gtest test/gtest_xml_output_unittest_.cc)
    gtest_py_test(gtest_xml_output_unittest --no_stacktrace_support)
    gtest_py_test(googletest-json-output-unittest --no_stacktrace_support)
endif()
