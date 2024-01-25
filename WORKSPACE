workspace(name = "com_google_googletest")

load("//:googletest_deps.bzl", "googletest_deps")
googletest_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "rules_python",
  sha256 = "d71d2c67e0bce986e1c5a7731b4693226867c45bfe0b7c5e0067228a536fc580",
  strip_prefix = "rules_python-0.29.0",
  urls = ["https://github.com/bazelbuild/rules_python/releases/download/0.29.0/rules_python-0.29.0.tar.gz"],
)

# https://github.com/bazelbuild/rules_python/releases/tag/0.29.0
load("@rules_python//python:repositories.bzl", "py_repositories")
py_repositories()

http_archive(
  name = "bazel_skylib",
  sha256 = "cd55a062e763b9349921f0f5db8c3933288dc8ba4f76dd9416aac68acee3cb94",
  urls = ["https://github.com/bazelbuild/bazel-skylib/releases/download/1.5.0/bazel-skylib-1.5.0.tar.gz"],
)

http_archive(
    name = "platforms",
    sha256 = "8150406605389ececb6da07cbcb509d5637a3ab9a24bc69b1101531367d89d74",
    urls = ["https://github.com/bazelbuild/platforms/releases/download/0.0.8/platforms-0.0.8.tar.gz"],
)
