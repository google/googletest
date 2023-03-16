workspace(name = "com_google_googletest")

load("//:googletest_deps.bzl", "googletest_deps")
googletest_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_python",  # 2023-01-10T22:00:51Z
    sha256 = "5de54486a60ad8948dabe49605bb1c08053e04001a431ab3e96745b4d97a4419",
    strip_prefix = "rules_python-70cce26432187a60b4e950118791385e6fb3c26f",
    urls = ["https://github.com/bazelbuild/rules_python/archive/70cce26432187a60b4e950118791385e6fb3c26f.zip"],
)

http_archive(
    name = "bazel_skylib",  # 2022-11-16T18:29:32Z
    sha256 = "a22290c26d29d3ecca286466f7f295ac6cbe32c0a9da3a91176a90e0725e3649",
    strip_prefix = "bazel-skylib-5bfcb1a684550626ce138fe0fe8f5f702b3764c3",
    urls = ["https://github.com/bazelbuild/bazel-skylib/archive/5bfcb1a684550626ce138fe0fe8f5f702b3764c3.zip"],
)

http_archive(
    name = "platforms",  # 2022-11-09T19:18:22Z
    sha256 = "b4a3b45dc4202e2b3e34e3bc49d2b5b37295fc23ea58d88fb9e01f3642ad9b55",
    strip_prefix = "platforms-3fbc687756043fb58a407c2ea8c944bc2fe1d922",
    urls = ["https://github.com/bazelbuild/platforms/archive/3fbc687756043fb58a407c2ea8c944bc2fe1d922.zip"],
)
