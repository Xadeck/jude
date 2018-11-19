load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

def dependencies():
    git_repository(
        name = "xdk_lua",
        commit = "de3b04f",
        remote = "https://github.com/Xadeck/lua.git",
    )

COPTS = [
    "-Wall",
    "-Werror",
]
