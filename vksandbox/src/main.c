#include <GLFW/glfw3.h>
#include <cglm/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#include "vlk_context.h"
#include "vlk_resources.h"
#include "vlk_swapchain.h"

#define SW_SLOTS 10

typedef struct {
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
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

  };
  const char** ext = (const char*[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkPhysicalDeviceVulkan13Features features13 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = NULL,
      .dynamicRendering = VK_TRUE,
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
  VkDescriptorSetLayoutBinding uboLayoutBinding = {
      .binding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
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

void createGraphicsPipeline(Vlk* vlk) {
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
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
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
      // TODO layouts are not created
      .pSetLayouts = &vlk->descriptorSetLayout,
  };
  vkCreatePipelineLayout(vlk->device, &pipelineLayoutInfo, NULL,
                         &vlk->pipelineLayout);
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
  };
  VkResult pipelineResult = vkCreateGraphicsPipelines(
      vlk->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &vlk->pipeline);
  if (pipelineResult != VK_SUCCESS) {
    printf("failed to create pipeline\n");
    exit(1);
  }
  vkDestroyShaderModule(vlk->device, sandboxFrag, NULL);
  vkDestroyShaderModule(vlk->device, sandboxVert, NULL);
}

void vlkInit(Vlk* vlk, GLFWwindow* window) {
  createInstance(vlk);
  pickPhysicalDevice(vlk);
  createLogicalDevice(vlk);
  vlkCreateSurface(vlk->instance, window, &vlk->surface);
  vlkCreateSwapchain(vlk->device, vlk->physicalDevice, vlk->extent, vlk->format,
                     vlk->surface, &vlk->swapchain);
  vlkGetSwapchainImages(vlk->device, vlk->swapchain, &vlk->swapchainImageCount,
                        vlk->swapchainImages);
  vlkCreateSwapchainImageViews(vlk->device, vlk->format,
                               vlk->swapchainImageCount, vlk->swapchainImages,
                               vlk->swapchainImageViews);
  createGraphicsPipeline(vlk);
}

void vlkDestroy(Vlk* vlk) {
  for (int i = 0; i < vlk->swapchainImageCount; i++) {
    vkDestroyImageView(vlk->device, vlk->swapchainImageViews[i], NULL);
  }
  vkDestroyPipeline(vlk->device, vlk->pipeline, NULL);
  vkDestroyPipelineLayout(vlk->device, vlk->pipelineLayout, NULL);
  vkDestroySwapchainKHR(vlk->device, vlk->swapchain, NULL);
  vkDestroySurfaceKHR(vlk->instance, vlk->surface, NULL);
  vkDestroyDevice(vlk->device, NULL);
  vkDestroyInstance(vlk->instance, NULL);
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
  vlkDestroy(&vlk);
  return 0;
}
