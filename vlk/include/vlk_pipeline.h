#ifndef VLK_PIPELINE
#define VLK_PIPELINE

#include <vulkan/vulkan_core.h>

void vlkCreateRenderPass(VkDevice device, VkFormat format,
                         VkSampleCountFlagBits msaaSample,
                         VkRenderPass* renderPass);

void vlkCreateDescriptorSetLayout(VkDevice device,
                                  VkDescriptorSetLayout* descriptorSetLayout);

void vlkCreatePipelineLayout(VkDevice device,
                             VkDescriptorSetLayout descriptorSetLayout,
                             VkPipelineLayout* pipelineLayout);

#endif  // !VLK_PIPELINE
