cc_library(
    name = "processor",
    srcs = ["processor.cc"],
    hdrs = ["processor.h"],
    copts = ["-Werror"],
    include_prefix = "xdk/ltemplate",
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_re2//:re2",
        "@lua",
    ],
)

cc_test(
    name = "processor_test",
    srcs = ["processor_test.cc"],
    copts = ["-Werror"],
    deps = [
        ":processor",
        "@com_google_googletest//:gtest_main",
        "@xdk_lua//:stack",
        "@xdk_lua//:state",
    ],
)
