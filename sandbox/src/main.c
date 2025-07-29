#include "engine.h"
#include "vulkan/vulkan_core.h"
#include <stdio.h>
#include <vulkan/vulkan.h>

int main() {
  VkInstance instance;
  createInstance(&instance);
  printf("Hello bazel\n");
  return 0;
}
