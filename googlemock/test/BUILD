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
    linkopts = select({
        "//:win": [],
        "//conditions:default": [
            "-pthread",
        ],
    }),
    deps = ["//:gtest"],
)
