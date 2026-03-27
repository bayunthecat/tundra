#include "vlk_resources.h"

#include <stdint.h>
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

uint32_t vlkFindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                           VkMemoryPropertyFlags props) {
  VkPhysicalDeviceMemoryProperties memProps = {};
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
  for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) &&
        (memProps.memoryTypes[i].propertyFlags & props) == props) {
      return i;
    }
  }
  printf("unable to find suitable memory\n");
  exit(1);
}

void vlkCreateImage(VkDevice device, VkPhysicalDevice physicalDevice,
                    uint32_t width, uint32_t height, uint32_t mipLevels,
                    VkSampleCountFlagBits numSamples, VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage,
                    VkMemoryPropertyFlags props, VkImage* image,
                    VkDeviceMemory* imageMemory) {
  VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .mipLevels = mipLevels,
      .samples = numSamples,
      .extent =
          {
              .width = width,
              .height = height,
              .depth = 1,
          },
      .arrayLayers = 1,
      .format = format,
      .tiling = tiling,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (vkCreateImage(device, &imageInfo, NULL, image) != VK_SUCCESS) {
    printf("image creation failed\n");
    exit(1);
  }
  VkMemoryRequirements memReq;
  vkGetImageMemoryRequirements(device, *image, &memReq);
  VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memReq.size,
      .memoryTypeIndex =
          vlkFindMemoryType(physicalDevice, memReq.memoryTypeBits,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
  if (vkAllocateMemory(device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
    printf("failed to allocate image memory\n");
    exit(1);
  }
  vkBindImageMemory(device, *image, *imageMemory, 0);
}

void vlkCreateImageView(VkDevice device, VkImage image, VkFormat format,
                        VkImageAspectFlags aspectFlags, uint32_t mipLevels,
                        VkImageView* imageView) {
  VkImageViewCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .subresourceRange =
          {
              .aspectMask = aspectFlags,
              .baseMipLevel = 0,
              .levelCount = mipLevels,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  if (vkCreateImageView(device, &info, NULL, imageView) != VK_SUCCESS) {
    printf("failed to create image view\n");
    exit(1);
  }
}

void vlkCreateShaderModule(VkDevice device, const char* filepath,
                           VkShaderModule* module) {
  printf("creating a shader module: %s\n", filepath);
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

void vlkCreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                     VkDeviceSize size, VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags props, VkBuffer* buffer,
                     VkDeviceMemory* memory) {
  VkBufferCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (vkCreateBuffer(device, &info, NULL, buffer) != VK_SUCCESS) {
    printf("error creating buffer\n");
    exit(1);
  }
  printf("creating buffer: %p\n", *buffer);
  VkMemoryRequirements memReq;
  vkGetBufferMemoryRequirements(device, *buffer, &memReq);
  VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memReq.size,
      .memoryTypeIndex =
          vlkFindMemoryType(physicalDevice, memReq.memoryTypeBits, props),
  };
  if (vkAllocateMemory(device, &allocInfo, NULL, memory) != VK_SUCCESS) {
    printf("error allocating memory\n");
    exit(1);
  };
  vkBindBufferMemory(device, *buffer, *memory, 0);
}
