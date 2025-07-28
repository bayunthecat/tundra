filegroup(
  name = "glfw_dll",
  srcs = ["glfw/lib-vc2022/glfw3.dll"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "glfw_win",
  includes = ["glfw/include"],
  deps = [":glfw_prebuilt_win"],
  visibility = ["//visibility:public"],
)

cc_import(
    name = "glfw_prebuilt_win",
    hdrs = glob(["glfw/include/GLFW/*.h"]),
    shared_library = "glfw/lib-vc2022/glfw3.dll",
    interface_library = "glfw/lib-vc2022/glfw3dll.lib",
    visibility = ["//visibility:private"],
)
