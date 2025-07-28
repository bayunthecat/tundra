#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

int main() {
  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .applicationVersion = 1,
      .apiVersion = VK_API_VERSION_1_4,
      .pApplicationName = "sandbox",
  };
  VkInstanceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = NULL,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = NULL,
  };
  VkInstance instance;
  if (vkCreateInstance(&info, NULL, &instance) != VK_SUCCESS) {
    printf("error createing vulkan instance\n");
    exit(1);
  }
  glfwInit();
  printf("Hello bazel\n");
  return 0;
}
