#include "vlk_resources.h"

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

// TODO needs a separate file and a module
void* vlkLoad(const char* filepath, size_t* size) {
  FILE* file = fopen(filepath, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", filepath);
    perror("");
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  *size = ftell(file);
  rewind(file);
  void* content = malloc(*size);
  if (content == NULL) {
    printf("malloc failed\n");
  }
  fread(content, 1, *size, file);
  fclose(file);
  return content;
}

void vlkCreateShaderModule(VkDevice device, const char* filepath,
                           VkShaderModule* module) {
  size_t size;
  uint32_t* code = vlkLoad(filepath, &size);
  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pCode = code,
      .codeSize = size,
  };
  VkResult result = vkCreateShaderModule(device, &info, NULL, module);
  if (result != VK_SUCCESS) {
    printf("failed to create shader module %s\n", filepath);
    exit(1);
  }
  free(code);
}
