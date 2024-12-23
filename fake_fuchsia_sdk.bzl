"""Provides a fake @fuchsia_sdk implementation that's used when the real one isn't available.

GoogleTest can be used with the [Fuchsia](https://fuchsia.dev/) SDK. However,
because the Fuchsia SDK does not yet support bzlmod, GoogleTest's `MODULE.bazel`
file by default provides a "fake" Fuchsia SDK.

To override this and use the real Fuchsia SDK, you can add the following to your
project's `MODULE.bazel` file:

    fake_fuchsia_sdk_extension =
    use_extension("@com_google_googletest//:fake_fuchsia_sdk.bzl", "fuchsia_sdk")
    override_repo(fake_fuchsia_sdk_extension, "fuchsia_sdk")

NOTE: The `override_repo` built-in is only available in Bazel 8.0 and higher.

See https://github.com/google/googletest/issues/4472 for more details of why the
fake Fuchsia SDK is needed.
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

_create_fake = tag_class()

def _fuchsia_sdk_impl(module_ctx):
    create_fake_sdk = False
    for mod in module_ctx.modules:
        for _ in mod.tags.create_fake:
            create_fake_sdk = True

    if create_fake_sdk:
        fake_fuchsia_sdk(name = "fuchsia_sdk")

    return module_ctx.extension_metadata(reproducible = True)

fuchsia_sdk = module_extension(
    implementation = _fuchsia_sdk_impl,
    tag_classes = {"create_fake": _create_fake},
)
