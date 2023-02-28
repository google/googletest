workspace(name = "com_google_googletest")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_absl",  # 2023-02-27T15:50:25Z
    sha256 = "baf8e734ac3ce213a889ce7c248b981ee1730e2093e32808e0f0a910dc985f76",
    strip_prefix = "abseil-cpp-0c1114c4fb83c844c7fd74708338cca1d3d9b0dc",
    urls = ["https://github.com/abseil/abseil-cpp/archive/0c1114c4fb83c844c7fd74708338cca1d3d9b0dc.zip"],
)

# Note this must use a commit from the `abseil` branch of the RE2 project.
# https://github.com/google/re2/tree/abseil
http_archive(
    name = "com_googlesource_code_re2",  # 2022-12-21T14:29:10Z
    sha256 = "b9ce3a51beebb38534d11d40f8928d40509b9e18a735f6a4a97ad3d014c87cb5",
    strip_prefix = "re2-d0b1f8f2ecc2ea74956c7608b6f915175314ff0e",
    urls = ["https://github.com/google/re2/archive/d0b1f8f2ecc2ea74956c7608b6f915175314ff0e.zip"],
)

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
