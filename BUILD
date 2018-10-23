COPTS = [
    "-Wall",
    "-Werror",
]

cc_library(
    name = "reader",
    srcs = ["reader.cc"],
    hdrs = ["reader.h"],
    copts = COPTS,
    include_prefix = "xdk/ltemplate",
    visibility = ["//visibility:private"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_re2//:re2",
        "@lua",
    ],
)

cc_test(
    name = "reader_test",
    srcs = ["reader_test.cc"],
    copts = COPTS,
    deps = [
        ":reader",
        "@com_google_googletest//:gtest_main",
        "@xdk_lua//:stack",
        "@xdk_lua//:state",
    ],
)
