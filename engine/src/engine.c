#include "engine.h"
#include "instance.h"
#include <GLFW/glfw3.h>
#include <cglm/affine-mat.h>
#include <cglm/affine.h>
#include <cglm/cglm.h>
#include <cglm/project.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define MAX_FRAMES_IN_FLIGHT 3

struct Engine {

  GLFWwindow *window;

  VkInstance instance;

  VkPhysicalDevice physicalDevice;

  VkDevice device;

  VkQueue queue;

  VkImage textureImage;

  VkImageView textureImageView;

  VkImage depthImage;

  VkImageView depthImageView;

  VkDeviceMemory depthImageMemory;

  VkSampler textureSampler;

  VkDeviceMemory textureMem;

  VkImage *swapchainImages;

  VkImageView *swapchainImageViews;

  VkPipelineLayout pipelineLayout;

  uint32_t imageCount;

  VkFormat swapchainImageFormat;

  VkExtent2D swapchainExtent;

  VkSwapchainKHR swapchain;

  VkSurfaceKHR surface;

  VkRenderPass renderPass;

  VkPipeline pipeline;

  VkFramebuffer *framebuffers;

  VkCommandPool commandPool;

  VkCommandBuffer *commandBuffers;

  VkSemaphore *imageAvailableSemaphores;

  VkSemaphore *renderFinishedSemaphores;

  VkFence *inFlight;

  VkBuffer vertexBuffer;

  VkDeviceMemory vertexBufferMemory;

  VkBuffer indexBuffer;

  VkDeviceMemory indexBufferMemory;

  VkBuffer *uniformBuffers;

  VkDeviceMemory *uniformBuffersMemoryList;

  void **uniformBuffersMapped;

  VkDescriptorSetLayout descriptorLayout;

  VkDescriptorPool descriptorPool;

  VkDescriptorSet *descriptorSets;

  uint32_t currentFrame;

  VkBuffer modelBuffer;

  VkDeviceMemory modelBufferMemory;

  vec3 *modelVerts;

  uint32_t modelVertsNum;

  vec2 *texCoords;

  uint32_t numTexCoords;

  int modelVerticesNum;

  uint32_t *modelIndices;

  uint32_t modelIndicesNum;

  VkBuffer modelIndiciesBuffer;

  VkDeviceMemory modelIndicesBufferMemory;

  VkSampleCountFlagBits msaaSample;

  VkImage colorImage;

  VkDeviceMemory colorImageMemory;

  VkImageView colorImageView;
};

// TODO mess

void createGlfw(Engine *e) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  e->window = glfwCreateWindow(800, 600, "Hello bazelized Vulkan", NULL, NULL);
}

void createSurface(Engine *e) {
  printf("creating surface\n");
  VkResult result =
      glfwCreateWindowSurface(e->instance, e->window, NULL, &e->surface);
  if (result != VK_SUCCESS) {
    printf("failed to create window surface, error code: %d\n", result);
    exit(1);
  }
}

void pickPhysicalDevice(Engine *e) {
  uint32_t deviceCount;
  vkEnumeratePhysicalDevices(e->instance, &deviceCount, NULL);
  if (deviceCount < 1) {
    printf("no compatible devices present\n");
    exit(1);
  }
  VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(e->instance, &deviceCount, devices);
  // skipped queue introspection logic for simplicity
  e->physicalDevice = devices[0];
  free(devices);
  printf("selected physical device: %p\n", e->physicalDevice);
}

void createLogicalDevice(Engine *e) {
  printf("creating logical device\n");
  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = 0,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};
  VkPhysicalDeviceFeatures features = {
      .samplerAnisotropy = VK_TRUE,
  };
  const char **ext = (const char *[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkDeviceCreateInfo deviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .pEnabledFeatures = &features,
      .enabledExtensionCount = 1,
      .ppEnabledExtensionNames = ext};
  VkResult createDeviceResult =
      vkCreateDevice(e->physicalDevice, &deviceCreateInfo, NULL, &e->device);
  if (createDeviceResult != VK_SUCCESS) {
    printf("failed to create logical device, error code: %d\n",
           createDeviceResult);
  }
}

void getDeviceQueues(Engine *e) {
  printf("getting device queue\n");
  vkGetDeviceQueue(e->device, 0, 0, &e->queue);
}

void createSwapchain(Engine *e) {
  printf("creating swapchain\n");
  VkSurfaceCapabilitiesKHR surfaceCaps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(e->physicalDevice, e->surface,
                                            &surfaceCaps);
  VkExtent2D extent = {
      .width = 800,
      .height = 600,
  };
  uint32_t swapchainImageCount = surfaceCaps.minImageCount + 1;
  VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  VkSwapchainCreateInfoKHR info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .imageFormat = imageFormat,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
      .imageExtent = extent,
      .surface = e->surface,
      .minImageCount = swapchainImageCount,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = 0,
      .preTransform = surfaceCaps.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };
  VkResult result = vkCreateSwapchainKHR(e->device, &info, NULL, &e->swapchain);
  if (result != VK_SUCCESS) {
    printf("failed to create swapchain\n");
    exit(1);
  }
  vkGetSwapchainImagesKHR(e->device, e->swapchain, &e->imageCount, NULL);
  if (e->imageCount == 0) {
    printf("no images in the swapchain\n");
    exit(1);
  }
  printf("swapchain images: %d\n", e->imageCount);
  e->swapchainImages = malloc(sizeof(VkImage) * e->imageCount);
  if (e->swapchainImages == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  vkGetSwapchainImagesKHR(e->device, e->swapchain, &e->imageCount,
                          e->swapchainImages);
  e->swapchainImageFormat = imageFormat;
  e->swapchainExtent = extent;
}

VkImageView createImageView(Engine *e, VkImage image, VkFormat format,
                            VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels) {
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
  VkImageView imageView;
  if (vkCreateImageView(e->device, &info, NULL, &imageView) != VK_SUCCESS) {
    printf("failed to create image view\n");
    exit(1);
  }
  return imageView;
}

void createImageViews(Engine *e) {
  printf("creating image views\n");
  e->swapchainImageViews = malloc(sizeof(VkImageView) * e->imageCount);
  if (e->swapchainImageViews == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  for (uint32_t i = 0; i < e->imageCount; i++) {
    e->swapchainImageViews[i] =
        createImageView(e, e->swapchainImages[i], e->swapchainImageFormat,
                        VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void createRenderPass(Engine *e) {
  printf("createing render pass\n");
  VkAttachmentDescription colorAttachment = {
      .format = e->swapchainImageFormat,
      .samples = e->msaaSample,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  VkAttachmentDescription depthAttachment = {
      .format = VK_FORMAT_D32_SFLOAT,
      .samples = e->msaaSample,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };
  VkAttachmentDescription colorAttachmentResolve = {
      .format = e->swapchainImageFormat,
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
      vkCreateRenderPass(e->device, &renderPassInfo, NULL, &e->renderPass);
  if (result != VK_SUCCESS) {
    printf("failed create render pass\n");
    exit(1);
  }
}

void createDescriptorSetLayout(Engine *e) {
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
  VkDescriptorSetLayoutBinding bindings[] = {
      uboLayoutBinding,
      samplerLayoutBinding,
  };
  VkDescriptorSetLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 2,
      .pBindings = bindings,
  };
  if (vkCreateDescriptorSetLayout(e->device, &info, NULL,
                                  &e->descriptorLayout) != VK_SUCCESS) {
    printf("Unable to create descriptor set layout\n");
    exit(1);
  };
}

VkVertexInputBindingDescription getVertexBindDesc() {
  VkVertexInputBindingDescription desc = {
      .binding = 0,
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      .stride = sizeof(Vertex),
  };
  return desc;
}

VkVertexInputAttributeDescription getVertexAttrDesc() {
  VkVertexInputAttributeDescription desc = {
      .binding = 0,
      .location = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(Vertex, vertex),
  };
  return desc;
}

VkVertexInputAttributeDescription getColorAttrDesc() {
  VkVertexInputAttributeDescription desc = {
      .binding = 0,
      .location = 1,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(Vertex, color),
  };
  return desc;
}

VkVertexInputAttributeDescription getTextureAttrDesc() {
  VkVertexInputAttributeDescription desc = {
      .binding = 0,
      .location = 2,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = offsetof(Vertex, texture),
  };
  return desc;
}

void *load(const char *filepath, size_t *size) {
  FILE *file = NULL;
  int ret = fopen_s(&file, filepath, "rb");
  if (ret != 0 || !file) {
    fprintf(stderr, "Failed to open file: %s, ret: %d\n", filepath, ret);
    perror("");
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  *size = ftell(file);
  rewind(file);
  void *content = malloc(*size);
  if (content == NULL) {
    printf("malloc failed\n");
  }
  fread(content, 1, *size, file);
  fclose(file);
  return content;
}

VkShaderModule createShaderModule(Engine *e, const char *filepath) {
  size_t size;
  uint32_t *code = load(filepath, &size);
  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pCode = code,
      .codeSize = size,
  };
  VkShaderModule module;
  VkResult result = vkCreateShaderModule(e->device, &info, NULL, &module);
  if (result != VK_SUCCESS) {
    printf("failed to create shader module %s\n", filepath);
    exit(1);
  }
  free(code);
  return module;
}

void createGraphicsPipeline(Engine *e) {
  VkShaderModule triVert =
      createShaderModule(e, "shaders/compiled/tri.vert.spv");
  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = triVert,
      .pName = "main",
  };
  VkShaderModule triFrag =
      createShaderModule(e, "shaders/compiled/tri.frag.spv");
  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = triFrag,
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
      getVertexAttrDesc(),
      getColorAttrDesc(),
      getTextureAttrDesc(),
  };
  VkVertexInputBindingDescription binds[] = {
      getVertexBindDesc(),
  };
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexAttributeDescriptionCount = 3,
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
      .width = (float)e->swapchainExtent.width,
      .height = (float)e->swapchainExtent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = e->swapchainExtent,
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
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,

  };
  VkPipelineMultisampleStateCreateInfo multisampleInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = e->msaaSample,
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
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pushConstantRangeCount = 0,
      .pSetLayouts = &e->descriptorLayout,
  };
  VkResult result = vkCreatePipelineLayout(e->device, &pipelineLayoutInfo, NULL,
                                           &e->pipelineLayout);
  if (result != VK_SUCCESS) {
    printf("failed pipeline layout\n");
    exit(1);
  }
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
      .layout = e->pipelineLayout,
      .renderPass = e->renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .pDepthStencilState = &depthStencilInfo,
  };
  VkResult pipelineResult = vkCreateGraphicsPipelines(
      e->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &e->pipeline);
  if (pipelineResult != VK_SUCCESS) {
    printf("failed to create pipeline\n");
    exit(1);
  }
  vkDestroyShaderModule(e->device, triFrag, NULL);
  vkDestroyShaderModule(e->device, triVert, NULL);
}

// TODO mess end

Engine *makeEngine() {
  Engine *e = malloc(sizeof(Engine));
  e->msaaSample = VK_SAMPLE_COUNT_8_BIT;
  createGlfw(e);
  createInstance(&e->instance);
  createSurface(e);
  pickPhysicalDevice(e);
  createLogicalDevice(e);
  getDeviceQueues(e);
  createSwapchain(e);
  createImageViews(e);
  createRenderPass(e);
  createDescriptorSetLayout(e);
  createGraphicsPipeline(e);
  // create command pool
  // create color resources
  // create depth resources
  // create framebuffers
  // create texture image
  // create texture image views
  // create texture sampler
  // create uniform buffers
  // create descriptor pool
  // create descriptor sets
  // create command buffers
  // create sync objects
  // create vertex buffer with models
  // create index buffer
  return e;
}

void destroySwapchainImages(Engine *e) {
  for (uint32_t i = 0; i < e->imageCount; i++) {
    // swapchain images are destroyed along with swapchain
    vkDestroyImageView(e->device, e->swapchainImageViews[i], NULL);
  }
  free(e->swapchainImages);
  free(e->swapchainImageViews);
}

void freeEngine(Engine *engine) {
  vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);
  destroySwapchainImages(engine);
  vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
  vkDestroyRenderPass(engine->device, engine->renderPass, NULL);
  vkDestroyPipelineLayout(engine->device, engine->pipelineLayout, NULL);
  vkDestroyPipeline(engine->device, engine->pipeline, NULL);
  vkDestroyDescriptorSetLayout(engine->device, engine->descriptorLayout, NULL);
  vkDestroyDevice(engine->device, NULL);
  vkDestroyInstance(engine->instance, NULL);
  glfwTerminate();
  free(engine);
}

void run(Engine *e) {
  while (!glfwWindowShouldClose(e->window)) {
    glfwPollEvents();
  }
}
