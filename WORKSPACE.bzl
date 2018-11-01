load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

def dependencies():
    git_repository(
        name = "xdk_lua",
        commit = "c6ae183",
        remote = "https://github.com/Xadeck/lua.git",
    )

    # TODO: google re2 has not yet tagged the abseil branch so a known
    # commit is used instead.
    git_repository(
        name = "com_google_re2",
        commit = "30b555b",
        remote = "https://github.com/google/re2.git",
    )

COPTS = [
    "-Wall",
    "-Werror",
]
