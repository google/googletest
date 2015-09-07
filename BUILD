# This Package contains the necessary targets to incorporate the googletest
# libraries into your Bazel built projects.
#
# The libraries are defined here rather than in their component directories
# because googlemock has dependencies on googletest and by defining the
# build targets here, the libraries are independent of their location in
# a more straightforward way.

licenses(["notice"])

cc_library(
    name = "googletest",
    srcs = glob([
    	"googletest/src/gtest.cc",
    	"googletest/src/gtest-death-test.cc",
    	"googletest/src/gtest-filepath.cc",
    	"googletest/src/gtest-internal-inl.h",
    	"googletest/src/gtest_main.cc",
    	"googletest/src/gtest-port.cc",
    	"googletest/src/gtest-printers.cc",
    	"googletest/src/gtest-test-part.cc",
    	"googletest/src/gtest-typed-test.cc",
    	"googletest/include/gtest/internal/*.h",
    	"googletest/include/gtest/internal/custom/*.h",
    ]),
    hdrs = glob([
    	"googletest/include/gtest/*.h"
    ]),
    includes = [
        "googletest",
        "googletest/include",
    ],
    copts = [
        "-g",
        "-Wall",
        "-Wextra",
<<<<<<< HEAD
=======
        "-pthread",
>>>>>>> Add support for Bazel build targets and fix some tests.
    ],
    linkopts = [
        "-pthread",
    ],
)

cc_library(
<<<<<<< HEAD
	name = "googlemock",
=======
    name = "googlemock",
>>>>>>> Add support for Bazel build targets and fix some tests.
    srcs = glob([
        "googlemock/src/gmock-cardinalities.cc",
        "googlemock/src/gmock.cc",
        "googlemock/src/gmock-internal-utils.cc",
        "googlemock/src/gmock_main.cc",
        "googlemock/src/gmock-matchers.cc",
        "googlemock/src/gmock-spec-builders.cc",
        "googlemock/include/gmock/internal/*.h",
        "googlemock/include/gmock/internal/custom/*.h",
    ]),
    hdrs = glob([
        "googlemock/include/gmock/*.h"
    ]),
    includes = [
<<<<<<< HEAD
        "googletest/include",
=======
>>>>>>> Add support for Bazel build targets and fix some tests.
        "googlemock",
        "googlemock/include",
    ],
    copts = [
        "-std=c++11",
        "-g",
        "-Wall",
        "-Wextra",
    ],
    linkopts = [
        "-pthread",
    ],
    deps = [
        ":googletest",
    ],
)

cc_library(
	name = "googletest_sample_libs",
	hdrs = [
	    "googletest/samples/prime_tables.h",
	    "googletest/samples/sample1.h",
	    "googletest/samples/sample2.h",
	    "googletest/samples/sample3-inl.h",
	    "googletest/samples/sample4.h",
	],
	srcs = [
	    "googletest/samples/sample1.cc",
	    "googletest/samples/sample2.cc",
	    "googletest/samples/sample4.cc",
	],
)

test_suite(
	name = "googletest_example_tests",
	tests = [
	    ":googletest_sample1_test",
	    ":googletest_sample2_test",
	    ":googletest_sample3_test",
	    ":googletest_sample4_test",
	    ":googletest_sample5_test",
	    ":googletest_sample6_test",
  	    ":googletest_sample7_test",
	    ":googletest_sample8_test",
	    ":googletest_sample9_test",
	    ":googletest_sample10_test",
	],
)

test_suite(
    name = "googletest_tests",
    tests = [
        ":googletest_gtest_filepath_test",
        ":googletest_gtest_linked_ptr_test",
        ":googletest_gtest_message_test",
        # The gtest_options_test fails when run as a test suite since it
        # uses a hard-coded binary name.
        # ":googletest_gtest_options_test",
        ":googletest_gtest_port_test",
        ":googletest_gtest_pred_impl_unittest",
        ":googletest_gtest_prod_test",
        ":googletest_gtest_test_part_test",
        ":googletest_gtest_typed_test_test",
        ":googletest_gtest_typed_test2_test",
        ":googletest_gtest_unittest",
        ":googletest_gtest_production_test",
    ],
)

test_suite(
    name = "googlemock_tests",
    tests = [
        ":googlemock_gmock_actions_test",
        ":googlemock_gmock_cardinalities_test",
        ":googlemock_gmock_ex_test",
        ":googlemock_gmock_generated_actions_test",
        ":googlemock_gmock-generated_function_mockers_test",
        ":googlemock_gmock-generated_internal_utils_test",
        ":googlemock_gmock-generated_matchers_test",
        ":googlemock_gmock_internal_utils_test",
        #  ":googlemock_gmock_leak_test",
        ":googlemock_gmock_link_test",
        ":googlemock_gmock_link2_test",
        ":googlemock_gmock_matchers_test",
        ":googlemock_gmock_more_actions_test",
        ":googlemock_gmock_nice_strict_test",
        #  ":googlemock_gmock_output_test",
        ":googlemock_gmock_port_test",
        ":googlemock_gmock_spec_builders_test",
        ":googlemock_gmock_stress_test",
        ":googlemock_gmock_test",
    ],
)

cc_test(
	name = "googletest_sample1_test",
	srcs = ["googletest/samples/sample1_unittest.cc"],
	deps = [
	    ":googletest_sample_libs",
	    ":googletest",
	],
)

cc_test(
	name = "googletest_sample2_test",
	srcs = ["googletest/samples/sample2_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample3_test",
	srcs = ["googletest/samples/sample3_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample4_test",
	srcs = ["googletest/samples/sample4_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample5_test",
	srcs = ["googletest/samples/sample5_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample6_test",
	srcs = ["googletest/samples/sample6_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample7_test",
	srcs = ["googletest/samples/sample7_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample8_test",
	srcs = ["googletest/samples/sample8_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample9_test",
	srcs = ["googletest/samples/sample9_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

cc_test(
	name = "googletest_sample10_test",
	srcs = ["googletest/samples/sample10_unittest.cc"],
	deps = [
	    ":googletest",
	    ":googletest_sample_libs",
	],
)

""" Tests on the googletest library itself."""
cc_library(
    name = "gtest_production",
    hdrs = [
        "googletest/test/production.h",
    ],
    srcs = [
        "googletest/test/production.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_filepath_test",
    srcs = [
        "googletest/test/gtest-filepath_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_linked_ptr_test",
    srcs = [
        "googletest/test/gtest-linked_ptr_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_message_test",
    srcs = [
        "googletest/test/gtest-message_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)

"""
This test currently fails based on how the test itself checks for the running executable
name.

cc_test(
    name = "googletest_gtest_options_test",
    srcs = [
        "googletest/test/gtest-options_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)"""

cc_test(
    name = "googletest_gtest_port_test",
    srcs = [
        "googletest/test/gtest-port_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_pred_impl_unittest",
    srcs = [
        "googletest/test/gtest_pred_impl_unittest.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_prod_test",
    srcs = [
        "googletest/test/gtest_prod_test.cc",
    ],
    includes = [
        "googletest",
        "googletest/include",
    ],
    deps = [
        ":googletest",
        ":gtest_production",
    ],
)

cc_test(
    name = "googletest_gtest_test_part_test",
    srcs = [
        "googletest/test/gtest-test-part_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_typed_test_test",
    srcs = [
        "googletest/test/gtest-typed-test_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_typed_test2_test",
    srcs = [
        "googletest/test/gtest-typed-test2_test.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_unittest",
    srcs = [
        "googletest/test/gtest_unittest.cc",
    ],
    deps = [
        ":googletest",
    ],
)

cc_test(
    name = "googletest_gtest_production_test",
    srcs = [
        "googletest/test/production.cc",
    ],
    deps = [
        ":googletest",
    ],
)

""" Tests on the googlemock library itself."""
cc_test(
    name = "googlemock_gmock_actions_test",
    srcs = [
        "googlemock/test/gmock-actions_test.cc",
    ],
    copts = [
        "-std=c++11",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_cardinalities_test",
    srcs = [
        "googlemock/test/gmock-cardinalities_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_ex_test",
    srcs = [
        "googlemock/test/gmock_ex_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_generated_actions_test",
    srcs = [
        "googlemock/test/gmock-generated-actions_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock-generated_function_mockers_test",
    srcs = [
        "googlemock/test/gmock-generated-function-mockers_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock-generated_internal_utils_test",
    srcs = [
        "googlemock/test/gmock-generated-internal-utils_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock-generated_matchers_test",
    srcs = [
        "googlemock/test/gmock-generated-matchers_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_internal_utils_test",
    srcs = [
        "googlemock/test/gmock-internal-utils_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

"""
This test currently runs through the python component and fails on its own.
cc_test(
    name = "googlemock_gmock_leak_test",
    srcs = [
        "googlemock/test/gmock_leak_test_.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)"""

cc_test(
    name = "googlemock_gmock_link_test",
    srcs = [
        "googlemock/test/gmock_link_test.h",
        "googlemock/test/gmock_link_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_link2_test",
    srcs = [
        "googlemock/test/gmock_link2_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_matchers_test",
    srcs = [
        "googlemock/test/gmock-matchers_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_more_actions_test",
    srcs = [
        "googlemock/test/gmock-more-actions_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_nice_strict_test",
    srcs = [
        "googlemock/test/gmock-nice-strict_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

"""
This test is currently run through the python component and fails on its own.
cc_test(
    name = "googlemock_gmock_output_test",
    srcs = [
        "googlemock/test/gmock_output_test_.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)"""

cc_test(
    name = "googlemock_gmock_port_test",
    srcs = [
        "googlemock/test/gmock-port_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_spec_builders_test",
    srcs = [
        "googlemock/test/gmock-spec-builders_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_stress_test",
    srcs = [
        "googlemock/test/gmock_stress_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)

cc_test(
    name = "googlemock_gmock_test",
    srcs = [
        "googlemock/test/gmock_test.cc",
    ],
    deps = [
        ":googlemock",
        ":googletest",
    ],
)
