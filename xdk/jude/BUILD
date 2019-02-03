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
        "@xdk_lua//xdk/lua:read",
        "@xdk_lua//xdk/lua:stack",
        "@xdk_lua//xdk/lua:state",
    ],
)

cc_library(
    name = "do",
    srcs = ["do.cc"],
    hdrs = ["do.h"],
    copts = COPTS,
    deps = [
        ":reader",
        "@xdk_lua//xdk/lua:back",
        "@xdk_lua//xdk/lua:sandbox",
    ],
)

cc_test(
    name = "do_test",
    srcs = ["do_test.cc"],
    copts = COPTS,
    deps = [
        ":do",
        "@com_google_googletest//:gtest_main",
        "@xdk_lua//xdk/lua:matchers",
        "@xdk_lua//xdk/lua:stack",
        "@xdk_lua//xdk/lua:state",
    ],
)
