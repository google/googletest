workspace(name = "com_google_googletest")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_absl",
    sha256 = "1a1745b5ee81392f5ea4371a4ca41e55d446eeaee122903b2eaffbd8a3b67a2b",
    strip_prefix = "abseil-cpp-01cc6567cff77738e416a7ddc17de2d435a780ce",
    urls = ["https://github.com/abseil/abseil-cpp/archive/01cc6567cff77738e416a7ddc17de2d435a780ce.zip"],  # 2022-06-21T19:28:27Z
)

# Note this must use a commit from the `abseil` branch of the RE2 project.
# https://github.com/google/re2/tree/abseil
http_archive(
    name = "com_googlesource_code_re2",
    sha256 = "0a890c2aa0bb05b2ce906a15efb520d0f5ad4c7d37b8db959c43772802991887",
    strip_prefix = "re2-a427f10b9fb4622dd6d8643032600aa1b50fbd12",
    urls = ["https://github.com/google/re2/archive/a427f10b9fb4622dd6d8643032600aa1b50fbd12.zip"],  # 2022-06-09
)

http_archive(
    name = "rules_python",
    sha256 = "0b460f17771258341528753b1679335b629d1d25e3af28eda47d009c103a6e15",
    strip_prefix = "rules_python-aef17ad72919d184e5edb7abf61509eb78e57eda",
    urls = ["https://github.com/bazelbuild/rules_python/archive/aef17ad72919d184e5edb7abf61509eb78e57eda.zip"],  # 2022-06-21T23:44:47Z
)

http_archive(
    name = "bazel_skylib",
    urls = ["https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz"],
    sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
)

http_archive(
    name = "platforms",
    sha256 = "a879ea428c6d56ab0ec18224f976515948822451473a80d06c2e50af0bbe5121",
    strip_prefix = "platforms-da5541f26b7de1dc8e04c075c99df5351742a4a2",
    urls = ["https://github.com/bazelbuild/platforms/archive/da5541f26b7de1dc8e04c075c99df5351742a4a2.zip"],  # 2022-05-27
)
