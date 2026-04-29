#include <GLFW/glfw3.h>
#include <bits/time.h>
#include <bits/types/stack_t.h>
#include <cglm/types.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <vulkan/vulkan_core.h>

#include "vlk_resources.h"
#include "vlk_swapchain.h"

#define SW_SLOTS 10

typedef float vec3[3];

typedef struct {
  int currentFrame;
  VkDeviceMemory vBufferMemoryList[SW_SLOTS];
  VkBuffer vBuffers[SW_SLOTS];
  void* vBufferMapped[SW_SLOTS];
  vec3* v;
  int vCount;
  struct timespec start;
} Render;

typedef struct {
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkQueue queue;
  VkDevice device;
  VkSurfaceKHR surface;
  VkFormat format;
  VkExtent2D extent;
  VkSwapchainKHR swapchain;
  uint32_t swapchainImageCount;
  VkImage swapchainImages[SW_SLOTS];
  VkImageView swapchainImageViews[SW_SLOTS];
  VkPipelineLayout pipelineLayout;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipeline pipeline;
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffers[SW_SLOTS];
  VkSemaphore acquireSemaphore[SW_SLOTS];
  VkSemaphore renderSemaphore[SW_SLOTS];
  VkFence inFlightFences[SW_SLOTS];
} Vlk;

void createInstance(Vlk* vlk) {
  uint32_t extCount = 0;
  const char** extensions = glfwGetRequiredInstanceExtensions(&extCount);
  uint32_t layersCount = 1;
  const char** layers = (const char*[]){"VK_LAYER_KHRONOS_validation"};
  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .apiVersion = VK_API_VERSION_1_4,
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
  };
  VkInstanceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = extCount,
      .ppEnabledExtensionNames = extensions,
      .enabledLayerCount = layersCount,
      .ppEnabledLayerNames = layers,
  };
  VkResult result = vkCreateInstance(&info, NULL, &vlk->instance);
  if (result != VK_SUCCESS) {
    printf("failed to create vulkan instance %d\n", result);
    exit(1);
  }
}

void pickPhysicalDevice(Vlk* vlk) {
  uint32_t deviceCount;
  vkEnumeratePhysicalDevices(vlk->instance, &deviceCount, NULL);
  if (deviceCount < 1) {
    printf("no compatible devices present\n");
    exit(1);
  }
  VkPhysicalDevice devices[deviceCount];
  vkEnumeratePhysicalDevices(vlk->instance, &deviceCount, devices);
  // skipped queue introspection logic for simplicity
  vlk->physicalDevice = devices[0];
  printf("selected physical device: %p\n", vlk->physicalDevice);
}

void createLogicalDevice(Vlk* vlk) {
  printf("creating logical device\n");
  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = 0,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority,
  };
  VkPhysicalDeviceFeatures features = {
      .samplerAnisotropy = VK_TRUE,
      .largePoints = VK_TRUE,
  };
  const char** ext = (const char*[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkPhysicalDeviceVulkan13Features features13 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = NULL,
      .dynamicRendering = VK_TRUE,
      .synchronization2 = VK_TRUE,
  };
  VkDeviceCreateInfo deviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .pEnabledFeatures = &features,
      .enabledExtensionCount = 1,
      .ppEnabledExtensionNames = ext,
      .pNext = &features13,
  };
  VkResult result = vkCreateDevice(vlk->physicalDevice, &deviceCreateInfo, NULL,
                                   &vlk->device);
  if (result != VK_SUCCESS) {
    printf("failed to create logical device, error code: %d\n", result);
  }
}

void createDescriptorSetLayout(VkDevice device,
                               VkDescriptorSetLayout* descriptorSetLayout) {
  printf("creating descriptor set layout\n");
  VkDescriptorSetLayoutBinding bindings[] = {};
  VkDescriptorSetLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 0,
      .pBindings = bindings,
  };
  if (vkCreateDescriptorSetLayout(device, &info, NULL, descriptorSetLayout) !=
      VK_SUCCESS) {
    printf("Unable to create descriptor set layout\n");
    exit(1);
  };
}

void createGraphicsPipeline(Vlk* vlk) {
  createDescriptorSetLayout(vlk->device, &vlk->descriptorSetLayout);
  VkShaderModule sandboxVert;
  vlkCreateShaderModule(
      vlk->device, "vksandbox/shaders/compiled/sandbox.vert.spv", &sandboxVert);
  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = sandboxVert,
      .pName = "main",
  };
  VkShaderModule sandboxFrag;
  vlkCreateShaderModule(
      vlk->device, "vksandbox/shaders/compiled/sandbox.frag.spv", &sandboxFrag);
  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = sandboxFrag,
      .pName = "main",
  };
  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};
  VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = dynamicStates,
  };
  VkVertexInputAttributeDescription attr[] = {
      {
          .binding = 0,
          .location = 0,
          .format = VK_FORMAT_R32G32B32_SFLOAT,
          .offset = 0,
      },
  };
  VkVertexInputBindingDescription binds[] = {
      {
          .binding = 0,
          .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
          .stride = sizeof(vec3),
      },
  };
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexAttributeDescriptionCount = 1,
      .pVertexAttributeDescriptions = attr,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = binds,
  };
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)vlk->extent.width,
      .height = (float)vlk->extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = vlk->extent,
  };
  VkPipelineViewportStateCreateInfo viewportStateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };
  VkPipelineRasterizationStateCreateInfo rasterInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .lineWidth = 1.0f,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,

  };
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      .blendEnable = VK_FALSE,
  };
  VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
  };
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
  };
  VkPipelineMultisampleStateCreateInfo multisampleInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = 1,
  };
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pushConstantRangeCount = 0,
      .pSetLayouts = &vlk->descriptorSetLayout,
  };
  vkCreatePipelineLayout(vlk->device, &pipelineLayoutInfo, NULL,
                         &vlk->pipelineLayout);
  VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .pNext = VK_NULL_HANDLE,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &vlk->format,
      .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
  };
  VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssemblyInfo,
      .pViewportState = &viewportStateInfo,
      .pRasterizationState = &rasterInfo,
      .pMultisampleState = &multisampleInfo,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicStateInfo,
      .layout = vlk->pipelineLayout,
      .renderPass = NULL,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .pDepthStencilState = &depthStencilInfo,
      .pNext = &pipelineRenderingCreateInfo,
  };
  VkResult pipelineResult = vkCreateGraphicsPipelines(
      vlk->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &vlk->pipeline);
  if (pipelineResult != VK_SUCCESS) {
    printf("failed to create pipeline\n");
    exit(1);
  }
  vkDestroyShaderModule(vlk->device, sandboxFrag, NULL);
  vkDestroyShaderModule(vlk->device, sandboxVert, NULL);
  printf("created graphics pipeline: %p\n", vlk->pipeline);
}

void createCommandPool(VkDevice device, VkCommandPool* pCommandPool) {
  VkCommandPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = NULL,
      .queueFamilyIndex = 0,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
  };
  vkCreateCommandPool(device, &info, NULL, pCommandPool);
}

void createCommandBuffers(VkDevice device, VkCommandPool commandPool, int count,
                          VkCommandBuffer* commandBuffers) {
  printf("creating command buffers\n");
  VkCommandBufferAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandPool = commandPool,
      .commandBufferCount = count,
  };
  vkAllocateCommandBuffers(device, &info, commandBuffers);
}

void createSyncObjects(VkDevice device, int count,
                       VkSemaphore* imageAvailableSemaphores,
                       VkSemaphore* renderFinishedSemaphores,
                       VkFence* inFlight) {
  VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  for (uint32_t i = 0; i < count; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, NULL,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS) {
      printf("failed to create semaphore\n");
      exit(1);
    }
    if (vkCreateSemaphore(device, &semaphoreInfo, NULL,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS) {
      printf("failed to create semaphore");
      exit(1);
    }
    if (vkCreateFence(device, &fenceInfo, NULL, &inFlight[i]) != VK_SUCCESS) {
      printf("failed to create fence");
      exit(1);
    }
  }
}

void copyBuffer(Vlk* vlk, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandPool = vlk->commandPool,
      .commandBufferCount = 1,
  };
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(vlk->device, &allocInfo, &commandBuffer);
  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
  if (result != VK_SUCCESS) {
    printf("failed to begin command buffer\n");
    exit(1);
  }
  VkBufferCopy copyRegion = {
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
  };
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
  vkEndCommandBuffer(commandBuffer);
  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
  };
  vkQueueSubmit(vlk->queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(vlk->queue);
  vkFreeCommandBuffers(vlk->device, vlk->commandPool, 1, &commandBuffer);
}

void createWritableVertexBuffer(Vlk* vlk, VkBuffer* buffer,
                                VkDeviceMemory* memory, VkDeviceSize size,
                                void** mapped) {
  vlkCreateBuffer(vlk->device, vlk->physicalDevice, size,
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  buffer, memory);
  vkMapMemory(vlk->device, *memory, 0, size, 0, mapped);
}

void createVertexBuffer(Vlk* vlk, VkBuffer* buffer, VkDeviceMemory* memory,
                        vec3* v, int vCount) {
  VkBuffer stgBuffer;
  VkDeviceMemory stgMemory;
  VkDeviceSize bufferSize = sizeof(vec3) * vCount;
  vlkCreateBuffer(vlk->device, vlk->physicalDevice, bufferSize,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stgBuffer, &stgMemory);

  void* data;
  vkMapMemory(vlk->device, stgMemory, 0, bufferSize, 0, &data);
  memcpy(data, v, bufferSize);
  vkUnmapMemory(vlk->device, stgMemory);
  vlkCreateBuffer(
      vlk->device, vlk->physicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);
  copyBuffer(vlk, stgBuffer, *buffer, bufferSize);
  vkDestroyBuffer(vlk->device, stgBuffer, NULL);
  vkFreeMemory(vlk->device, stgMemory, NULL);
}

void vlkInit(Vlk* vlk, GLFWwindow* window) {
  createInstance(vlk);
  pickPhysicalDevice(vlk);
  createLogicalDevice(vlk);
  vkGetDeviceQueue(vlk->device, 0, 0, &vlk->queue);
  vlkCreateSurface(vlk->instance, window, &vlk->surface);
  vlkCreateSwapchain(vlk->device, vlk->physicalDevice, vlk->extent, vlk->format,
                     vlk->surface, &vlk->swapchain);
  vlkGetSwapchainImages(vlk->device, vlk->swapchain, &vlk->swapchainImageCount,
                        vlk->swapchainImages);
  vlkCreateSwapchainImageViews(vlk->device, vlk->format,
                               vlk->swapchainImageCount, vlk->swapchainImages,
                               vlk->swapchainImageViews);
  createGraphicsPipeline(vlk);
  createCommandPool(vlk->device, &vlk->commandPool);
  createCommandBuffers(vlk->device, vlk->commandPool, vlk->swapchainImageCount,
                       vlk->commandBuffers);
  createSyncObjects(vlk->device, vlk->swapchainImageCount,
                    vlk->acquireSemaphore, vlk->renderSemaphore,
                    vlk->inFlightFences);
}

void destroySemaphores(VkDevice device, int count, VkSemaphore* semaphores) {
  for (int i = 0; i < count; i++) {
    vkDestroySemaphore(device, semaphores[i], NULL);
  }
}

void destroyFences(VkDevice device, int count, VkFence* fences) {
  for (int i = 0; i < count; i++) {
    vkDestroyFence(device, fences[i], NULL);
  }
}

void vlkDestroy(Vlk* vlk) {
  for (int i = 0; i < vlk->swapchainImageCount; i++) {
    vkDestroyImageView(vlk->device, vlk->swapchainImageViews[i], NULL);
  }
  destroySemaphores(vlk->device, vlk->swapchainImageCount,
                    vlk->acquireSemaphore);
  destroySemaphores(vlk->device, vlk->swapchainImageCount,
                    vlk->renderSemaphore);
  destroyFences(vlk->device, vlk->swapchainImageCount, vlk->inFlightFences);
  vkDestroyCommandPool(vlk->device, vlk->commandPool, NULL);
  vkDestroyDescriptorSetLayout(vlk->device, vlk->descriptorSetLayout, NULL);
  vkDestroyPipeline(vlk->device, vlk->pipeline, NULL);
  vkDestroyPipelineLayout(vlk->device, vlk->pipelineLayout, NULL);
  vkDestroySwapchainKHR(vlk->device, vlk->swapchain, NULL);
  vkDestroySurfaceKHR(vlk->instance, vlk->surface, NULL);
  vkDestroyDevice(vlk->device, NULL);
  vkDestroyInstance(vlk->instance, NULL);
}

void toPresent(Vlk* vlk, VkCommandBuffer commandBuffer, int swImageIndex,
               int currentFrame) {
  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
      .dstAccessMask = VK_ACCESS_2_NONE,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .image = vlk->swapchainImages[swImageIndex],
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  VkDependencyInfo dep = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(commandBuffer, &dep);
}

void toColorAttachment(Vlk* vlk, VkCommandBuffer commandBuffer,
                       int swImageIndex, int currentFrame) {
  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
      .srcAccessMask = VK_ACCESS_2_NONE,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = vlk->swapchainImages[swImageIndex],
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  VkDependencyInfo dep = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(commandBuffer, &dep);
}

void recordCommandBuffer(Vlk* vlk, VkCommandBuffer commandBuffer,
                         VkPipeline pipeline, VkImageView swapchainImageView,
                         Render* render, int swImageIndex) {
  vkResetCommandBuffer(commandBuffer, 0);
  VkRenderingAttachmentInfo colorAttachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = swapchainImageView,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue =
          {
              .color =
                  {
                      .float32 = {0.0f, 0.0f, 0.0f, 1.0f},
                  },
          },
  };
  VkRenderingInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea =
          {
              .offset = {0, 0},
              .extent = {800, 800},
          },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
  };
  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  toColorAttachment(vlk, commandBuffer, swImageIndex, render->currentFrame);
  vkCmdBeginRendering(commandBuffer, &renderingInfo);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = vlk->extent.width,
      .height = vlk->extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  VkOffset2D offset = {0};
  VkRect2D scissor = {
      .extent = vlk->extent,
      .offset = offset,
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &render->vBuffers[swImageIndex],
                         offsets);
  vkCmdDraw(commandBuffer, render->vCount, 1, 0, 0);
  vkCmdEndRendering(commandBuffer);
  toPresent(vlk, commandBuffer, swImageIndex, render->currentFrame);
  vkEndCommandBuffer(commandBuffer);
}

void present(Vlk* vlk, int swImageIndex, int currentFrame) {
  const uint32_t imgIdx = (uint32_t)swImageIndex;
  VkPresentInfoKHR info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pWaitSemaphores = &vlk->renderSemaphore[swImageIndex],
      .waitSemaphoreCount = 1,
      .pSwapchains = &vlk->swapchain,
      .swapchainCount = 1,
      .pImageIndices = &imgIdx,
  };
  VkResult result = vkQueuePresentKHR(vlk->queue, &info);
  if (result != VK_SUCCESS) {
    printf("present failed: %d\n", result);
    exit(1);
  }
}

void submit(Vlk* vlk, int swImageIndex, int currentFrame) {
  VkCommandBufferSubmitInfo commandBufferInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
      .commandBuffer = vlk->commandBuffers[currentFrame],
  };
  VkSemaphoreSubmitInfo waitInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = vlk->acquireSemaphore[currentFrame],
      .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
  };
  VkSemaphoreSubmitInfo signalInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = vlk->renderSemaphore[swImageIndex],
      .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
  };
  VkSubmitInfo2 submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &commandBufferInfo,
      .waitSemaphoreInfoCount = 1,
      .pWaitSemaphoreInfos = &waitInfo,
      .signalSemaphoreInfoCount = 1,
      .pSignalSemaphoreInfos = &signalInfo,
  };
  VkResult result = vkQueueSubmit2(vlk->queue, 1, &submitInfo,
                                   vlk->inFlightFences[currentFrame]);
  if (result != VK_SUCCESS) {
    printf("submit failed: %d\n", result);
    exit(1);
  }
}

static inline double sec(struct timespec t) {
  return t.tv_sec + (double)t.tv_nsec / 1e9;
}

static inline double dt(struct timespec curr, struct timespec start) {
  return sec(curr) - sec(start);
}

static inline void rotateXY(vec3 p, vec3 origin, float angle) {
  double rads = angle * (M_PI / 180.0f);
  float x = origin[0];
  float y = origin[1];
  p[0] = x * cos(rads) - y * sin(rads);
  p[1] = x * sin(rads) + y * cos(rads);
  p[2] = origin[2];
}

static inline void rotateXZ(vec3 p, vec3 origin, float angle) {
  double rads = angle * (M_PI / 180.0f);
  float x = origin[0];
  float z = origin[2];
  p[0] = x * cos(rads) - z * sin(rads);
  p[1] = origin[1];
  p[2] = x * sin(rads) + z * cos(rads);
}

static inline void project(vec3 p, vec3 origin, float dz) {
  p[0] = origin[0] / dz;
  p[1] = origin[1] / dz;
  p[2] = origin[2] + dz;
}

void updateBuffers(Render* render, void* vertices) {
  struct timespec curr;
  clock_gettime(CLOCK_MONOTONIC, &curr);
  double t = dt(curr, render->start);
  float speed = 0.2f;
  float rotateSpeed = 180.0f;
  vec3* v = vertices;
  float dz = speed * t;
  float da = rotateSpeed * t;
  for (int i = 0; i < render->vCount; i++) {
    rotateXZ(v[i], render->v[i], da);
    rotateXY(v[i], v[i], da);
    project(v[i], v[i], dz);
  }
}

void draw(Vlk* vlk, Render* render) {
  vkWaitForFences(vlk->device, 1, &vlk->inFlightFences[render->currentFrame],
                  VK_TRUE, UINT64_MAX);
  vkResetFences(vlk->device, 1, &vlk->inFlightFences[render->currentFrame]);

  uint32_t swImageIndex;
  vkAcquireNextImageKHR(vlk->device, vlk->swapchain, UINT64_MAX,
                        vlk->acquireSemaphore[render->currentFrame],
                        VK_NULL_HANDLE, &swImageIndex);
  updateBuffers(render, render->vBufferMapped[swImageIndex]);
  recordCommandBuffer(vlk, vlk->commandBuffers[render->currentFrame],
                      vlk->pipeline, vlk->swapchainImageViews[swImageIndex],
                      render, swImageIndex);
  submit(vlk, swImageIndex, render->currentFrame);
  present(vlk, swImageIndex, render->currentFrame);
  render->currentFrame = (render->currentFrame + 1) % vlk->swapchainImageCount;
}

void createWritableVBuffers(Vlk* vlk, Render* render) {
  VkDeviceSize size = sizeof(vec3) * render->vCount;
  for (int i = 0; i < vlk->swapchainImageCount; i++) {
    createWritableVertexBuffer(vlk, &render->vBuffers[i],
                               &render->vBufferMemoryList[i], size,
                               &render->vBufferMapped[i]);
    memcpy(render->vBufferMapped[i], render->v, size);
  }
}

void freeBuffers(VkDevice device, VkBuffer* buffers, VkDeviceMemory* memories,
                 int count) {
  for (int i = 0; i < count; i++) {
    vkDestroyBuffer(device, buffers[i], NULL);
    vkFreeMemory(device, memories[i], NULL);
  }
}

void mainLoop(Vlk* vlk, GLFWwindow* window) {
  vec3 v[] = {
      {0.5f, 0.5f, 0.5f},    {-0.5f, 0.5f, 0.5f},
      {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},

      {0.5f, 0.5f, -0.5f},   {-0.5f, 0.5f, -0.5f},
      {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},

  };
  Render render = {
      .currentFrame = 0,
      .vCount = 8,
      .v = v,
  };
  createWritableVBuffers(vlk, &render);
  clock_gettime(CLOCK_MONOTONIC, &render.start);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    draw(vlk, &render);
    glfwSwapBuffers(window);
  }
  vkDeviceWaitIdle(vlk->device);
  freeBuffers(vlk->device, render.vBuffers, render.vBufferMemoryList,
              vlk->swapchainImageCount);
}

int main() {
  Vlk vlk;
  vlk.extent = (VkExtent2D){800, 800};
  vlk.format = VK_FORMAT_B8G8R8A8_UNORM;
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(vlk.extent.width, vlk.extent.height,
                                        "sandbox", NULL, NULL);
  vlkInit(&vlk, window);
  mainLoop(&vlk, window);
  vlkDestroy(&vlk);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
