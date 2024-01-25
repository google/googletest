"""Load dependencies needed to use the googletest library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def googletest_deps():
    """Loads common dependencies needed to use the googletest library."""

    if not native.existing_rule("com_googlesource_code_re2"):
        http_archive(
            name = "com_googlesource_code_re2",
            sha256 = "828341ad08524618a626167bd320b0c2acc97bd1c28eff693a9ea33a7ed2a85f",
            strip_prefix = "re2-2023-11-01",
            urls = ["https://github.com/google/re2/releases/download/2023-11-01/re2-2023-11-01.zip"],
        )

    if not native.existing_rule("com_google_absl"):
        http_archive(
            name = "com_google_absl",
            sha256 = "338420448b140f0dfd1a1ea3c3ce71b3bc172071f24f4d9a57d59b45037da440",
            strip_prefix = "abseil-cpp-20240116.0",
            urls = ["https://github.com/abseil/abseil-cpp/releases/download/20240116.0/abseil-cpp-20240116.0.tar.gz"],
        )
