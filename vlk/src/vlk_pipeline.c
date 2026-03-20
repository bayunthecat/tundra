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

void vlkCreateGraphicsPipeline(VkDevice device, VkPipelineLayout pipelineLayout,
                               VkPipeline pipeline) {
  // VkShaderModule triVert =
  //     createShaderModule(e, "shaders/compiled/tri.vert.spv");
  // VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
  //     .stage = VK_SHADER_STAGE_VERTEX_BIT,
  //     .module = triVert,
  //     .pName = "main",
  // };
  // VkShaderModule triFrag =
  //     createShaderModule(e, "shaders/compiled/tri.frag.spv");
  // VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
  //     .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
  //     .module = triFrag,
  //     .pName = "main",
  // };
  // VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
  //                                                   fragShaderStageInfo};
  // VkDynamicState dynamicStates[] = {
  //     VK_DYNAMIC_STATE_VIEWPORT,
  //     VK_DYNAMIC_STATE_SCISSOR,
  // };
  // VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
  //     .dynamicStateCount = 2,
  //     .pDynamicStates = dynamicStates,
  // };
  // VkVertexInputAttributeDescription attr[] = {
  //     getVertexAttrDesc(),
  //     getColorAttrDesc(),
  //     getTextureAttrDesc(),
  // };
  // VkVertexInputBindingDescription binds[] = {
  //     getVertexBindDesc(),
  // };
  // VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  //     .vertexAttributeDescriptionCount = 3,
  //     .pVertexAttributeDescriptions = attr,
  //     .vertexBindingDescriptionCount = 1,
  //     .pVertexBindingDescriptions = binds,
  // };
  // VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
  //     .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  //     .primitiveRestartEnable = VK_FALSE,
  // };
  // VkViewport viewport = {
  //     .x = 0.0f,
  //     .y = 0.0f,
  //     .width = (float)e->swapchainExtent.width,
  //     .height = (float)e->swapchainExtent.height,
  //     .minDepth = 0.0f,
  //     .maxDepth = 1.0f,
  // };
  // VkRect2D scissor = {
  //     .offset = {0, 0},
  //     .extent = e->swapchainExtent,
  // };
  // VkPipelineViewportStateCreateInfo viewportStateInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
  //     .viewportCount = 1,
  //     .pViewports = &viewport,
  //     .scissorCount = 1,
  //     .pScissors = &scissor,
  // };
  // VkPipelineRasterizationStateCreateInfo rasterInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
  //     .depthClampEnable = VK_FALSE,
  //     .rasterizerDiscardEnable = VK_FALSE,
  //     .polygonMode = VK_POLYGON_MODE_FILL,
  //     .lineWidth = 1.0f,
  //     .cullMode = VK_CULL_MODE_NONE,
  //     .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
  //     .depthBiasEnable = VK_FALSE,
  //
  // };
  // VkPipelineMultisampleStateCreateInfo multisampleInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
  //     .sampleShadingEnable = VK_FALSE,
  //     .rasterizationSamples = e->msaaSample,
  // };
  // VkPipelineColorBlendAttachmentState colorBlendAttachment = {
  //     .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
  //                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  //     .blendEnable = VK_FALSE,
  // };
  // VkPipelineColorBlendStateCreateInfo colorBlending = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
  //     .logicOpEnable = VK_FALSE,
  //     .logicOp = VK_LOGIC_OP_COPY,
  //     .attachmentCount = 1,
  //     .pAttachments = &colorBlendAttachment,
  //     .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
  // };
  // VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
  //     .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
  //     .depthTestEnable = VK_TRUE,
  //     .depthWriteEnable = VK_TRUE,
  //     .depthCompareOp = VK_COMPARE_OP_LESS,
  //     .depthBoundsTestEnable = VK_FALSE,
  //     .stencilTestEnable = VK_FALSE,
  // };
  //
  // VkGraphicsPipelineCreateInfo pipelineInfo = {
  //     .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
  //     .stageCount = 2,
  //     .pStages = shaderStages,
  //     .pVertexInputState = &vertexInputInfo,
  //     .pInputAssemblyState = &inputAssemblyInfo,
  //     .pViewportState = &viewportStateInfo,
  //     .pRasterizationState = &rasterInfo,
  //     .pMultisampleState = &multisampleInfo,
  //     .pColorBlendState = &colorBlending,
  //     .pDynamicState = &dynamicStateInfo,
  //     .layout = e->pipelineLayout,
  //     .renderPass = e->renderPass,
  //     .subpass = 0,
  //     .basePipelineHandle = VK_NULL_HANDLE,
  //     .pDepthStencilState = &depthStencilInfo,
  // };
  // VkResult pipelineResult = vkCreateGraphicsPipelines(
  //     e->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &e->pipeline);
  // if (pipelineResult != VK_SUCCESS) {
  //   printf("failed to create pipeline\n");
  //   exit(1);
  // }
  // vkDestroyShaderModule(e->device, triFrag, NULL);
  // vkDestroyShaderModule(e->device, triVert, NULL);
}
