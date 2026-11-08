# Copyright 2024 Google Inc.
# All Rights Reserved.
#
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

workspace(name = "googletest")

load("//:googletest_deps.bzl", "googletest_deps")

googletest_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_python",
    sha256 = "4765b9a4a9c6e0fb89f465aac5b5b0ac06589c9477144340859310777ad801a7",
    strip_prefix = "rules_python-2.4.6",
    url = "https://github.com/bazel-contrib/rules_python/releases/download/2.4.6/rules_python-2.4.6.tar.gz",
)

# https://github.com/bazelbuild/rules_python/releases/tag/1.1.0
load("@rules_python//python:repositories.bzl", "py_repositories")

py_repositories()

http_archive(
    name = "platforms",
    sha256 = "dbad4a23abcca6171e47b79edc53bd6a41067a3b75f9e8b104656b459ff25046",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/platforms/releases/download/1.1.0/platforms-1.1.0.tar.gz",
        "https://github.com/bazelbuild/platforms/releases/download/1.1.0/platforms-1.1.0.tar.gz",
    ],
)

load("@bazel_features//:deps.bzl", "bazel_features_deps")

bazel_features_deps()

load("@rules_cc//cc:extensions.bzl", "compatibility_proxy_repo")

compatibility_proxy_repo()

load("@bazel_tools//tools/build_defs/repo:local.bzl", "local_repository")

local_repository(
    name = "docker_toolchain",
    path = "ci/docker_toolchain",
)
