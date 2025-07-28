cc_library(
  name = "vulkan_win",
  hdrs = glob(["vulkan-sdk/Include/vulkan/*.h"]),
  includes = ["vulkan-sdk/Include"],
  deps = [":vulkan_lib_win"],
  visibility = ["//visibility:public"],
)

cc_import(
    name = "vulkan_lib_win", 
    interface_library = "vulkan-sdk/Lib/vulkan-1.lib",
    visibility = ["//visibility:private"],
    system_provided = True,
)
