# TODO: Fix has_absl default to true for bzlmod

## Remaining Steps:
1. [x] Edit BUILD.bazel: Add bzlmod_enabled config_setting and update gtest defines/deps selects.
2. [x] Edit docs/quickstart-bazel.md: Add note about default Abseil support with bzlmod.
3. [x] Test changes: bazel test //... --enable_bzlmod=true succeeded.
4. attempt_completion.

Progress will be updated as steps complete.

