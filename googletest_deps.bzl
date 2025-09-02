"""Load dependencies needed to use the googletest library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//:fake_fuchsia_sdk.bzl", "fake_fuchsia_sdk")

def googletest_deps():
    """Loads common dependencies needed to use the googletest library."""

    if not native.existing_rule("re2"):
        http_archive(
            name = "re2",
            sha256 = "eb2df807c781601c14a260a507a5bb4509be1ee626024cb45acbd57cb9d4032b",
            strip_prefix = "re2-2024-07-02",
            urls = ["https://github.com/google/re2/releases/download/2024-07-02/re2-2024-07-02.tar.gz"],
        )

    if not native.existing_rule("abseil-cpp"):
        http_archive(
            name = "abseil-cpp",
            sha256 = "9b2b72d4e8367c0b843fa2bcfa2b08debbe3cee34f7aaa27de55a6cbb3e843db",
            strip_prefix = "abseil-cpp-20250814.0",
            urls = ["https://github.com/abseil/abseil-cpp/releases/download/20250814.0/abseil-cpp-20250814.0.tar.gz"],
        )

    if not native.existing_rule("fuchsia_sdk"):
        fake_fuchsia_sdk(
            name = "fuchsia_sdk",
        )
