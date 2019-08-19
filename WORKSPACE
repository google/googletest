workspace(name = "com_google_googletest")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Abseil
http_archive(
      name = "com_google_absl",
      urls = ["https://github.com/abseil/abseil-cpp/archive/d9aa92d7fb324314f9df487ac23d32a25650b742.zip"],  # 2019-08-13T18:21:13Z
      strip_prefix = "abseil-cpp-d9aa92d7fb324314f9df487ac23d32a25650b742",
      sha256 = "caf4c323eb6211397df96dd5ff96e46c7e5dd77c74d3daed2181f87868159eca",
)

http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-master",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/master.zip"],
)

http_archive(
    name = "rules_python",
    strip_prefix = "rules_python-master",
    urls = ["https://github.com/bazelbuild/rules_python/archive/master.zip"],
)

