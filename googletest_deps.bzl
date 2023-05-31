"""Load dependencies needed to use the googletest library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def googletest_deps():
    """Loads common dependencies needed to use the googletest library."""

    if not native.existing_rule("com_googlesource_code_re2"):
        http_archive(
            name = "com_googlesource_code_re2",  # 2023-06-01
            sha256 = "1726508efc93a50854c92e3f7ac66eb28f0e57652e413f11d7c1e28f97d997ba",
            strip_prefix = "re2-03da4fc0857c285e3a26782f6bc8931c4c950df4",
            urls = ["https://github.com/google/re2/archive/03da4fc0857c285e3a26782f6bc8931c4c950df4.zip"],
        )

    if not native.existing_rule("com_google_absl"):
        http_archive(
            name = "com_google_absl",  # 2023-04-06T14:42:25Z
            sha256 = "a50452f02402262f9a61a8eedda60f76dda6b9538d36b34b55bce9f74a4d5ef8",
            strip_prefix = "abseil-cpp-e73b9139ee9b853a4bd7812531442c138da09084",
            urls = ["https://github.com/abseil/abseil-cpp/archive/e73b9139ee9b853a4bd7812531442c138da09084.zip"],
        )
