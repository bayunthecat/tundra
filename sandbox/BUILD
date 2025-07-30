cc_binary(
  name = "main",
  srcs = glob(["src/*.c"]),
  visibility = ["//visibility:public"],
  deps = [
    "//engine:engine",
  ],
  data = [
    "//tp/glfw:glfw_dll",
    "//shaders:shaders",
  ],
)
