cc_binary(
  name = "main",
  srcs = glob(["src/*.c"]),
  visibility = ["//visibility:public"],
  deps = [
    "//tp/glfw:glfw_win", 
    "//tp/vulkan-sdk:vulkan_win",
  ],
  data = ["//tp/glfw:glfw_dll"],
)
