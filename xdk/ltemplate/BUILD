load("//:WORKSPACE.bzl", "COPTS")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "reader",
    srcs = ["reader.cc"],
    hdrs = ["reader.h"],
    copts = COPTS,
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
        "@xdk_lua//xdk/lua:stack",
        "@xdk_lua//xdk/lua:state",
    ],
)

cc_library(
    name = "ltemplate",
    srcs = ["ltemplate.cc"],
    hdrs = ["ltemplate.h"],
    copts = COPTS,
    deps = [
        ":reader",
        "@xdk_lua//xdk/lua:back",
        "@xdk_lua//xdk/lua:sandbox",
    ],
)

cc_test(
    name = "ltemplate_test",
    srcs = [
        "ltemplate_test.cc",
    ],
    copts = COPTS,
    deps = [
        ":ltemplate",
        "@com_google_googletest//:gtest_main",
        "@xdk_lua//xdk/lua:matchers",
        "@xdk_lua//xdk/lua:stack",
        "@xdk_lua//xdk/lua:state",
    ],
)
