#ifndef VLK_PIPELINE
#define VLK_PIPELINE

#include <vulkan/vulkan_core.h>

void vlkCreateRenderPass(VkDevice device, VkFormat format,
                         VkSampleCountFlagBits msaaSample,
                         VkRenderPass* renderPass);

#endif  // !VLK_PIPELINE
