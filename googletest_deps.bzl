"""Load dependencies needed to use the googletest library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def googletest_deps():
    """Loads common dependencies needed to use the googletest library."""

    if not native.existing_rule("com_googlesource_code_re2"):
        http_archive(
            name = "com_googlesource_code_re2",  # 2023-03-17T11:36:51Z
            sha256 = "cb8b5312a65f2598954545a76e8bce913f35fbb3a21a5c88797a4448e9f9b9d9",
            strip_prefix = "re2-578843a516fd1da7084ae46209a75f3613b6065e",
            urls = ["https://github.com/google/re2/archive/578843a516fd1da7084ae46209a75f3613b6065e.zip"],
        )

    if not native.existing_rule("com_google_absl"):
        http_archive(
            name = "com_google_absl",  # 2023-09-13T14:58:42Z
            sha256 = "7766815ef6293dc7bca58fef59a96d7d3230874412dcd36dafb0e313ed1356f2",
            strip_prefix = "abseil-cpp-9e1789ffea47fdeb3133aa42aa9592f3673fb6ed",
            urls = ["https://github.com/abseil/abseil-cpp/archive/9e1789ffea47fdeb3133aa42aa9592f3673fb6ed.zip"],
        )
