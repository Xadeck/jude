load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_google_absl",
    remote = "https://github.com/abseil/abseil-cpp.git",
    tag = "20180600",
)

# TODO: google re2 has not yet tagged the abseil branch so a known
# commit is used instead.
git_repository(
    name = "com_google_re2",
    commit = "30b555b",
    remote = "https://github.com/google/re2.git",
)

git_repository(
    name = "xdk_lua",
    commit = "c08159b",
    remote = "https://github.com/Xadeck/lua.git",
)

load("@xdk_lua//:WORKSPACE.bzl", xdk_lua_dependencies = "dependencies")

xdk_lua_dependencies()
