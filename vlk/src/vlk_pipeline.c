#include "vlk_pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

void vlkCreateRenderPass(VkDevice device, VkFormat format,
                         VkSampleCountFlagBits msaaSample,
                         VkRenderPass* renderPass) {
  printf("creating render pass\n");
  VkAttachmentDescription colorAttachment = {
      .format = format,
      .samples = msaaSample,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  VkAttachmentDescription depthAttachment = {
      .format = VK_FORMAT_D32_SFLOAT,
      .samples = msaaSample,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
  VkAttachmentDescription colorAttachmentResolve = {
      .format = format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  VkAttachmentReference colorAttachmentRef = {
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  VkAttachmentReference depthAttachmentRef = {
      .attachment = 1,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
  VkAttachmentReference colorAttachmentResolveRef = {
      .attachment = 2,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  VkSubpassDescription subpass = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
      .pDepthStencilAttachment = &depthAttachmentRef,
      .pResolveAttachments = &colorAttachmentResolveRef,
  };
  VkSubpassDependency dependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  };
  VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment,
                                           colorAttachmentResolve};
  VkRenderPassCreateInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 3,
      .pAttachments = attachments,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,

  };
  VkResult result =
      vkCreateRenderPass(device, &renderPassInfo, NULL, renderPass);
  if (result != VK_SUCCESS) {
    printf("failed create render pass\n");
    exit(1);
  }
}

void vlkCreateDescriptorSetLayout(VkDevice device,
                                  VkDescriptorSetLayout* descriptorSetLayout) {
  // TODO probably requires futher splits
  printf("creating descriptor set layout\n");
  VkDescriptorSetLayoutBinding uboLayoutBinding = {
      .binding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
  };
  VkDescriptorSetLayoutBinding samplerLayoutBinding = {
      .binding = 1,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImmutableSamplers = NULL,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
  };
  VkDescriptorSetLayoutBinding ssboLayoutBinding = {
      .binding = 2,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
  };
  VkDescriptorSetLayoutBinding bindings[] = {
      uboLayoutBinding,
      samplerLayoutBinding,
      ssboLayoutBinding,
  };
  VkDescriptorSetLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 3,
      .pBindings = bindings,
  };
  if (vkCreateDescriptorSetLayout(device, &info, NULL, descriptorSetLayout) !=
      VK_SUCCESS) {
    printf("Unable to create descriptor set layout\n");
    exit(1);
  };
}

void vlkCreatePipelineLayout(VkDevice device,
                             VkDescriptorSetLayout descriptorSetLayout,
                             VkPipelineLayout* pipelineLayout) {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pushConstantRangeCount = 0,
      .pSetLayouts = &descriptorSetLayout,
  };
  VkResult result =
      vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, pipelineLayout);
  if (result != VK_SUCCESS) {
    printf("failed pipeline layout\n");
    exit(1);
  }
}
