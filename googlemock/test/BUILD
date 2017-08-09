# Copyright 2017 Google Inc. All Rights Reserved.
# Author: misterg@google.com (Gennadiy Civil)
#
# Description:
#   Bazel BUILD file for googletest-googlemock, initial revision
#

""" gmock own tests """

cc_test(
    name = "gmock_all_test",
    size = "small",
    srcs = glob(
        include = [
            "gmock-*.cc",
        ],
    ),
    copts = select({
        "//:win": ["-DGTEST_USE_OWN_TR1_TUPLE=0"],
        "//conditions:default": ["-DGTEST_USE_OWN_TR1_TUPLE=1"],
    }),
    linkopts = select({
        "//:win": [],
        "//conditions:default": [
            "-pthread",
        ],
    }),
    deps = ["//:gtest"],
)
