"""Load dependencies needed to use the googletest library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def googletest_deps():
    """Loads common dependencies needed to use the googletest library."""

    if not native.existing_rule("com_googlesource_code_re2"):
        http_archive(
            name = "com_googlesource_code_re2",  # 2022-12-21T14:29:10Z
            sha256 = "b9ce3a51beebb38534d11d40f8928d40509b9e18a735f6a4a97ad3d014c87cb5",
            strip_prefix = "re2-d0b1f8f2ecc2ea74956c7608b6f915175314ff0e",
            urls = ["https://github.com/google/re2/archive/d0b1f8f2ecc2ea74956c7608b6f915175314ff0e.zip"],
        )

    if not native.existing_rule("com_google_absl"):
        http_archive(
            name = "com_google_absl",  # 2023-02-27T15:50:25Z
            sha256 = "baf8e734ac3ce213a889ce7c248b981ee1730e2093e32808e0f0a910dc985f76",
            strip_prefix = "abseil-cpp-0c1114c4fb83c844c7fd74708338cca1d3d9b0dc",
            urls = ["https://github.com/abseil/abseil-cpp/archive/0c1114c4fb83c844c7fd74708338cca1d3d9b0dc.zip"],
        )
