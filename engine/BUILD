cc_library(
  name = "engine",
  srcs = glob(["src/*.c"]),
  hdrs = glob(["include/*.h"]),
  includes = ["include"],
  deps = [
    "//tp/glfw:glfw_win", 
    "//tp/vulkan-sdk:vulkan_win",
  ],
  visibility = ["//visibility:public"],
)
