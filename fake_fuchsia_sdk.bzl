"""Provides a fake @fuchsia_sdk implementation that's used when the real one isn't available.

This is needed since bazel queries on targets that depend on //:gtest (eg:
`bazel query "deps(set(//googletest/test:gtest_all_test))"`) will fail if @fuchsia_sdk is not
defined when bazel is evaluating the transitive closure of the query target.

See https://github.com/google/googletest/issues/4472.
"""

def _fake_fuchsia_sdk_impl(repo_ctx):
    for stub_target in repo_ctx.attr._stub_build_targets:
        stub_package = stub_target
        stub_target_name = stub_target.split("/")[-1]
        repo_ctx.file("%s/BUILD.bazel" % stub_package, """
filegroup(
    name = "%s",
)
""" % stub_target_name)

fake_fuchsia_sdk = repository_rule(
    doc = "Used to create a fake @fuchsia_sdk repository with stub build targets.",
    implementation = _fake_fuchsia_sdk_impl,
    attrs = {
        "_stub_build_targets": attr.string_list(
            doc = "The stub build targets to initialize.",
            default = [
                "pkg/fdio",
                "pkg/syslog",
                "pkg/zx",
            ],
        ),
    },
)
