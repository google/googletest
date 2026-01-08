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
            sha256 = "4c124408da902be896a2f368042729655709db5e3004ec99f57e3e14439bc1b2",
            strip_prefix = "abseil-cpp-20260107.0",
            urls = ["https://github.com/abseil/abseil-cpp/releases/download/20260107.0/abseil-cpp-20260107.0.tar.gz"],
        )

    if not native.existing_rule("bazel_features"):
        http_archive(
            name = "bazel_features",
            sha256 = "9390b391a68d3b24aef7966bce8556d28003fe3f022a5008efc7807e8acaaf1a",
            strip_prefix = "bazel_features-1.36.0",
            url = "https://github.com/bazel-contrib/bazel_features/releases/download/v1.36.0/bazel_features-v1.36.0.tar.gz",
        )

    if not native.existing_rule("rules_cc"):
        http_archive(
            name = "rules_cc",
            sha256 = "207ea073dd20a705f9e8bc5ac02f5203e9621fc672774bb1a0935aefab7aebfa",
            strip_prefix = "rules_cc-0.2.8",
            url = "https://github.com/bazelbuild/rules_cc/releases/download/0.2.8/rules_cc-0.2.8.tar.gz",
        )

    if not native.existing_rule("fuchsia_sdk"):
        fake_fuchsia_sdk(
            name = "fuchsia_sdk",
        )
