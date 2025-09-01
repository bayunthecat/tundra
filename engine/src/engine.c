#include <cglm/cam.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "engine.h"
#include "stb_image.h"
#include "tinyobj_loader_c.h"
#include <GLFW/glfw3.h>
#include <cglm/affine-mat.h>
#include <cglm/affine.h>
#include <cglm/cglm.h>
#include <cglm/project.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define MAX_FRAMES_IN_FLIGHT 4

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

  Vertex *modelVertices;

  int modelVerticesNum;

  vec2 *texCoords;

  uint32_t numTexCoords;

  uint32_t *modelIndices;

  uint32_t modelIndicesNum;

  VkBuffer modelIndiciesBuffer;

  VkDeviceMemory modelIndicesBufferMemory;

  VkSampleCountFlagBits msaaSample;

  VkImage colorImage;

  VkDeviceMemory colorImageMemory;

  VkImageView colorImageView;

  uint32_t mipLevels;

  clock_t start;
};

// TODO mess

void createInstance(VkInstance *instance) {
  uint32_t extCount = 0;
  const char **extensions = glfwGetRequiredInstanceExtensions(&extCount);
  uint32_t layersCount = 1;
  const char **layers = (const char *[]){"VK_LAYER_KHRONOS_validation"};
  printf("extension count: %d\n", extCount);
  for (int i = 0; i < extCount; i++) {
    printf("ext: %d, %s\n", i, extensions[i]);
  }
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
  VkResult result = vkCreateInstance(&info, NULL, instance);
  if (result != VK_SUCCESS) {
    printf("failed to create vulkan instance %d\n", result);
    exit(1);
  }
}

void createGlfw(Engine *e) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  e->window = glfwCreateWindow(800, 600, "Hello Vulkan", NULL, NULL);
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

uint32_t findMemoryType(Engine *e, uint32_t typeFilter,
                        VkMemoryPropertyFlags props) {
  VkPhysicalDeviceMemoryProperties memProps = {};
  vkGetPhysicalDeviceMemoryProperties(e->physicalDevice, &memProps);
  for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) &&
        (memProps.memoryTypes[i].propertyFlags & props) == props) {
      return i;
    }
  }
  printf("unable to find suitable memory\n");
  exit(1);
}

void createImage(Engine *e, uint32_t width, uint32_t height, uint32_t mipLevels,
                 VkSampleCountFlagBits numSamples, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags props, VkImage *image,
                 VkDeviceMemory *imageMemory) {
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
  if (vkCreateImage(e->device, &imageInfo, NULL, image) != VK_SUCCESS) {
    printf("image creation failed\n");
    exit(1);
  }
  VkMemoryRequirements memReq;
  vkGetImageMemoryRequirements(e->device, *image, &memReq);

  VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memReq.size,
      .memoryTypeIndex = findMemoryType(e, memReq.memoryTypeBits,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
  if (vkAllocateMemory(e->device, &allocInfo, NULL, imageMemory) !=
      VK_SUCCESS) {
    printf("failed to allocate image memory\n");
    exit(1);
  }
  vkBindImageMemory(e->device, *image, *imageMemory, 0);
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
  printf("creating render pass\n");
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
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", filepath);
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

void createCommandPool(Engine *e) {
  printf("creating command pool\n");
  VkCommandPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = 0,
  };
  VkResult result =
      vkCreateCommandPool(e->device, &info, NULL, &e->commandPool);
  if (result != VK_SUCCESS) {
    printf("failed to create command pool");
    exit(1);
  }
}

void createColorResources(Engine *e) {
  printf("creating color resources\n");
  VkFormat colorFormat = e->swapchainImageFormat;
  createImage(e, e->swapchainExtent.width, e->swapchainExtent.height, 1,
              e->msaaSample, colorFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &e->colorImage,
              &e->colorImageMemory);
  e->colorImageView = createImageView(e, e->colorImage, colorFormat,
                                      VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void createDepthResources(Engine *e) {
  printf("creating depth resources\n");
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
  createImage(e, e->swapchainExtent.width, e->swapchainExtent.height, 1,
              e->msaaSample, depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &e->depthImage,
              &e->depthImageMemory);
  e->depthImageView = createImageView(e, e->depthImage, depthFormat,
                                      VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void createFramebuffers(Engine *e) {
  printf("creating framebuffers\n");
  e->framebuffers = malloc(sizeof(VkFramebuffer) * e->imageCount);
  if (e->framebuffers == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  for (uint32_t i = 0; i < e->imageCount; i++) {
    VkImageView attachments[] = {
        e->colorImageView,
        e->depthImageView,
        e->swapchainImageViews[i],
    };
    VkFramebufferCreateInfo framebufferInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = e->renderPass,
        .attachmentCount = 3,
        .pAttachments = attachments,
        .width = e->swapchainExtent.width,
        .height = e->swapchainExtent.height,
        .layers = 1,
    };
    VkResult result = vkCreateFramebuffer(e->device, &framebufferInfo, NULL,
                                          &e->framebuffers[i]);
    if (result != VK_SUCCESS) {
      printf("failed to create framebuffer");
      exit(1);
    }
  }
}

void createBuffer(Engine *e, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags props, VkBuffer *buffer,
                  VkDeviceMemory *memory) {
  VkBufferCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (vkCreateBuffer(e->device, &info, NULL, buffer) != VK_SUCCESS) {
    printf("error creating buffer\n");
    exit(1);
  }
  VkMemoryRequirements memReq;
  vkGetBufferMemoryRequirements(e->device, *buffer, &memReq);

  VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memReq.size,
      .memoryTypeIndex = findMemoryType(e, memReq.memoryTypeBits, props),
  };
  if (vkAllocateMemory(e->device, &allocInfo, NULL, memory) != VK_SUCCESS) {
    printf("error allocating memory\n");
    exit(1);
  };
  vkBindBufferMemory(e->device, *buffer, *memory, 0);
}

VkCommandBuffer beginSingleTimeCommands(Engine *e) {
  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandPool = e->commandPool,
      .commandBufferCount = 1,
  };
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(e->device, &allocInfo, &commandBuffer);
  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void endSingleTimeCommands(Engine *e, VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);
  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
  };
  vkQueueSubmit(e->queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(e->queue);
  vkFreeCommandBuffers(e->device, e->commandPool, 1, &commandBuffer);
}

void transitionImageLayout(Engine *e, VkImage image, VkFormat format,
                           VkImageLayout oldLayout, VkImageLayout newLayout,
                           uint32_t mipLevels) {
  VkCommandBuffer cmdBuff = beginSingleTimeCommands(e);
  VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = mipLevels,
          .baseArrayLayer = 0,
          .layerCount = 1,
      }};
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;
  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    printf("unsupported ransistion\n");
    exit(1);
  }
  vkCmdPipelineBarrier(cmdBuff, sourceStage, destinationStage, 0, 0, NULL, 0,
                       NULL, 1, &barrier);
  endSingleTimeCommands(e, cmdBuff);
}

void copyBufferToImage(Engine *e, VkBuffer buffer, VkImage image,
                       uint32_t width, uint32_t height) {
  VkCommandBuffer cmdBuff = beginSingleTimeCommands(e);
  VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1},
      .imageSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  vkCmdCopyBufferToImage(cmdBuff, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  endSingleTimeCommands(e, cmdBuff);
}

void generateMipmaps(Engine *e, VkImage image, uint32_t texWidth,
                     uint32_t texHeight, uint32_t mipLevels) {
  printf("generateMipmaps\n");
  // Skip checking physical device format capabilities
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(e);
  VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .image = image,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseArrayLayer = 0,
              .layerCount = 1,
              .levelCount = 1,
          },
  };
  uint32_t mipWidth = texWidth;
  uint32_t mipHeight = texHeight;
  for (uint32_t i = 1; i < mipLevels; i++) {
    printf("mipHeight %d, mipWidth: %d\n", mipHeight, mipWidth);
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &barrier);
    VkImageBlit blit = {
        .srcOffsets[0] = {0, 0, 0},
        .srcOffsets[1] = {mipWidth, mipHeight, 1},
        .srcSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .dstOffsets[0] = {0, 0, 0},
        .dstOffsets[1] =
            {
                mipWidth > 1 ? mipWidth / 2 : 1,
                mipHeight > 1 ? mipHeight / 2 : 1,
                1,
            },
        .dstSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                         NULL, 1, &barrier);
    if (mipWidth > 1) {
      mipWidth /= 2;
    }
    if (mipHeight > 1) {
      mipHeight /= 2;
    }
  }
  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                       NULL, 1, &barrier);
  endSingleTimeCommands(e, commandBuffer);
}

void createTextureImage(Engine *e) {
  printf("creating texture image\n");
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load("assets/viking_room.png", &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  e->mipLevels = ((uint32_t)floor(log2(fmax(texWidth, texHeight)))) + 1;
  VkDeviceSize dSize = texWidth * texHeight * 4;
  VkBuffer stage;
  VkDeviceMemory stageMem;
  createBuffer(e, dSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
               &stage, &stageMem);
  void *data;
  vkMapMemory(e->device, stageMem, 0, dSize, 0, &data);
  memcpy(data, pixels, dSize);
  vkUnmapMemory(e->device, stageMem);
  free(pixels);

  createImage(e, texWidth, texHeight, e->mipLevels, VK_SAMPLE_COUNT_1_BIT,
              VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
              &e->textureImage, &e->textureMem);
  transitionImageLayout(e, e->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, e->mipLevels);
  copyBufferToImage(e, stage, e->textureImage, texWidth, texHeight);
  generateMipmaps(e, e->textureImage, texWidth, texHeight, e->mipLevels);
}

void createTextureImageView(Engine *e) {
  printf("creating texture image view\n");
  e->textureImageView =
      createImageView(e, e->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                      VK_IMAGE_ASPECT_COLOR_BIT, e->mipLevels);
}

void createTextureSampler(Engine *e) {
  printf("creating texture sampler\n");
  VkPhysicalDeviceProperties props = {};
  vkGetPhysicalDeviceProperties(e->physicalDevice, &props);
  VkSamplerCreateInfo samplerInfo = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = props.limits.maxSamplerAnisotropy,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .minLod = 0.0f,
      .maxLod = (float)e->mipLevels,
      .mipLodBias = 0.0f,
  };
  if (vkCreateSampler(e->device, &samplerInfo, NULL, &e->textureSampler) !=
      VK_SUCCESS) {
    printf("error creating sampler\n");
    exit(1);
  }
}

void createUniformBuffers(Engine *e) {
  printf("creating uniform buffers\n");
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);
  e->uniformBuffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
  if (e->uniformBuffers == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  e->uniformBuffersMemoryList =
      malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
  if (e->uniformBuffersMemoryList == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  e->uniformBuffersMapped = malloc(sizeof(void *) * MAX_FRAMES_IN_FLIGHT);
  if (e->uniformBuffersMapped == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(e, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &e->uniformBuffers[i], &e->uniformBuffersMemoryList[i]);
    vkMapMemory(e->device, e->uniformBuffersMemoryList[i], 0, bufferSize, 0,
                &e->uniformBuffersMapped[i]);
  }
}

void createDescriptorPool(Engine *e) {
  printf("creating descriptor pool\n");
  VkDescriptorPoolSize poolSize = {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT,
  };
  VkDescriptorPoolSize samplerPoolSize = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = MAX_FRAMES_IN_FLIGHT,
  };
  VkDescriptorPoolSize poolSizes[] = {
      poolSize,
      samplerPoolSize,
  };
  VkDescriptorPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .poolSizeCount = 2,
      .pPoolSizes = poolSizes,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
  };
  if (vkCreateDescriptorPool(e->device, &info, NULL, &e->descriptorPool) !=
      VK_SUCCESS) {
    printf("failed create descriptor pool\n");
    exit(1);
  };
}

void createDescriptorSets(Engine *e) {
  printf("creating descriptor sets\n");
  VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    layouts[i] = e->descriptorLayout;
  }
  e->descriptorSets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
  if (e->descriptorSets == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  VkDescriptorSetAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = e->descriptorPool,
      .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
      .pSetLayouts = layouts,
  };
  if (vkAllocateDescriptorSets(e->device, &info, e->descriptorSets) !=
      VK_SUCCESS) {
    printf("Unable to allocate descriptor sets\n");
    exit(1);
  }
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo = {
        .buffer = e->uniformBuffers[i],
        .offset = 0,
        .range = sizeof(UniformBufferObject),
    };
    VkWriteDescriptorSet bufferWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = e->descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .pBufferInfo = &bufferInfo,
    };
    VkDescriptorImageInfo imageInfo = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView = e->textureImageView,
        .sampler = e->textureSampler,
    };
    VkWriteDescriptorSet samplerWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = e->descriptorSets[i],
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &imageInfo};

    VkWriteDescriptorSet writes[] = {bufferWrite, samplerWrite};
    vkUpdateDescriptorSets(e->device, 2, writes, 0, NULL);
  }
}

void createCommandBuffers(Engine *e) {
  e->commandBuffers = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer));
  if (e->commandBuffers == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  VkCommandBufferAllocateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = e->commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
  };
  VkResult result =
      vkAllocateCommandBuffers(e->device, &info, e->commandBuffers);
  if (result != VK_SUCCESS) {
    printf("failed to allocate buffers\n");
    exit(1);
  }
}

void createSyncObjects(Engine *e) {
  printf("creating sync objects\n");
  e->imageAvailableSemaphores =
      malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
  if (e->imageAvailableSemaphores == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  e->renderFinishedSemaphores =
      malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
  if (e->renderFinishedSemaphores == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  e->inFlight = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
  if (e->inFlight == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    printf("creating image available semaphore: %d\n", i);
    if (vkCreateSemaphore(e->device, &semaphoreInfo, NULL,
                          &e->imageAvailableSemaphores[i]) != VK_SUCCESS) {
      printf("failed to create semaphore\n");
      exit(1);
    }
    printf("creating render finished semaphore: %d\n", i);
    if (vkCreateSemaphore(e->device, &semaphoreInfo, NULL,
                          &e->renderFinishedSemaphores[i]) != VK_SUCCESS) {
      printf("failed to create semaphore");
      exit(1);
    }
    printf("creating in flight fence: %d\n", i);
    if (vkCreateFence(e->device, &fenceInfo, NULL, &e->inFlight[i]) !=
        VK_SUCCESS) {
      printf("failed to create fence");
      exit(1);
    }
  }
}

void copyBuffer(Engine *e, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandPool = e->commandPool,
      .commandBufferCount = 1,
  };
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(e->device, &allocInfo, &commandBuffer);
  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(commandBuffer, &beginInfo);
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
  vkQueueSubmit(e->queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(e->queue);
  vkFreeCommandBuffers(e->device, e->commandPool, 1, &commandBuffer);
}

void loadFile(void *ctx, const char *filename, const int isMtl,
              const char *objFilename, char **data, size_t *len) {
  if (isMtl == 1) {
    return;
  }
  *data = load(filename, len);
  printf("load file len: %ld\n", *len);
}

void loadModel(const char *filename, Vertex **vertices, int *numVertices) {
  printf("loading model: %s\n", filename);
  tinyobj_attrib_t attrib;
  tinyobj_shape_t *shapes;
  tinyobj_material_t *materials;
  size_t numShapes;
  size_t numMaterials;
  int result =
      tinyobj_parse_obj(&attrib, &shapes, &numShapes, &materials, &numMaterials,
                        filename, loadFile, NULL, TINYOBJ_FLAG_TRIANGULATE);
  if (result != TINYOBJ_SUCCESS) {
    printf("Failed to load model, error: %d\n", result);
    exit(1);
  }
  vec3 verts[attrib.num_vertices];
  for (uint32_t i = 0; i < attrib.num_vertices; i++) {
    verts[i][0] = attrib.vertices[i * 3 + 0];
    verts[i][1] = attrib.vertices[i * 3 + 1];
    verts[i][2] = attrib.vertices[i * 3 + 2];
  }
  vec2 texCoords[attrib.num_texcoords];
  for (uint32_t i = 0; i < attrib.num_texcoords; i++) {
    texCoords[i][0] = attrib.texcoords[i * 2 + 0];
    texCoords[i][1] = 1.0f - attrib.texcoords[i * 2 + 1];
  }

  Vertex *v = malloc(sizeof(Vertex) * attrib.num_faces);
  if (v == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  *numVertices = attrib.num_faces;
  int r = 0;
  int count = 0;
  for (uint32_t i = 0; i < attrib.num_face_num_verts; i++) {
    for (uint32_t j = 0; j < attrib.face_num_verts[r]; j++) {
      uint32_t faceIdx = (i * 3) + j;
      tinyobj_vertex_index_t f = attrib.faces[faceIdx];
      memcpy(&v[count].vertex, &verts[f.v_idx], sizeof(vec3));
      memcpy(&v[count].texture, &texCoords[f.vt_idx], sizeof(vec2));
      count++;
    }
    r++;
  }
  *vertices = v;
}

void updateUniformBuffer(Engine *e, uint32_t currentImage) {
  if (e->start == 0) {
    e->start = clock();
  }
  clock_t currentTime = clock();
  float time = (float)(currentTime - e->start) / CLOCKS_PER_SEC;
  UniformBufferObject ubo = {
      .model = GLM_MAT4_IDENTITY_INIT,
      .view = GLM_MAT4_IDENTITY_INIT,
      .proj = GLM_MAT4_IDENTITY_INIT,
  };
  // TODO do not rotate for now
  // glm_rotate(ubo.model, time * glm_rad(45.0f), (vec3){0.0f, 1.0f, 0.0f});
  vec3 eye = {0.0f, 5.0f, 0.0f};
  vec3 center = {0.0f, 0.0f, 0.0f};
  vec3 up = {0.0f, 0.0f, 1.0f};
  glm_lookat(eye, center, up, ubo.view);
  glm_perspective(glm_rad(100.0f),
                  e->swapchainExtent.width / (float)e->swapchainExtent.height,
                  0.1f, 10.0f, ubo.proj);
  ubo.proj[1][1] *= -1;
  memcpy(e->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void recordCommandBuffer(Engine *e, VkCommandBuffer commandBuffer,
                         uint32_t imageIndex) {
  VkCommandBufferBeginInfo begingInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };
  VkResult result = vkBeginCommandBuffer(commandBuffer, &begingInfo);
  if (result != VK_SUCCESS) {
    printf("failed go begin command buffer");
    exit(1);
  }
  VkClearValue clearColor = {
      .color = {{0.0f, 0.0f, 0.0f, 1.0f}},
  };
  VkClearValue clearDepth = {
      .depthStencil = {1.0f, 0},
  };
  VkClearValue clears[] = {clearColor, clearDepth};
  VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = e->renderPass,
      .framebuffer = e->framebuffers[imageIndex],
      .renderArea.offset = {0, 0},
      .renderArea.extent = e->swapchainExtent,
      .pClearValues = clears,
      .clearValueCount = 2,
  };
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    e->pipeline);
  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = e->swapchainExtent.width,
      .height = e->swapchainExtent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = e->swapchainExtent,
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &e->modelBuffer, offsets);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          e->pipelineLayout, 0, 1,
                          &e->descriptorSets[e->currentFrame], 0, NULL);
  vkCmdDraw(commandBuffer, e->modelVerticesNum, 1, 0, 0);
  vkCmdEndRenderPass(commandBuffer);
  VkResult endBufferResult = vkEndCommandBuffer(commandBuffer);
  if (endBufferResult != VK_SUCCESS) {
    printf("end buffer failed\n");
    exit(1);
  }
}

void drawFrame(Engine *e) {
  vkWaitForFences(e->device, 1, &e->inFlight[e->currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(e->device, e->swapchain, UINT64_MAX,
                        e->imageAvailableSemaphores[e->currentFrame],
                        VK_NULL_HANDLE, &imageIndex);
  updateUniformBuffer(e, e->currentFrame);
  vkResetFences(e->device, 1, &e->inFlight[e->currentFrame]);
  vkResetCommandBuffer(e->commandBuffers[e->currentFrame], 0);
  recordCommandBuffer(e, e->commandBuffers[e->currentFrame], imageIndex);

  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &e->imageAvailableSemaphores[e->currentFrame],
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &e->commandBuffers[e->currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &e->renderFinishedSemaphores[e->currentFrame],
  };
  if (vkQueueSubmit(e->queue, 1, &submitInfo, e->inFlight[e->currentFrame]) !=
      VK_SUCCESS) {
    printf("queue submit failed\n");
    exit(1);
  }
  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &e->renderFinishedSemaphores[e->currentFrame],
      .swapchainCount = 1,
      .pSwapchains = &e->swapchain,
      .pImageIndices = &imageIndex,
  };
  VkResult result = vkQueuePresentKHR(e->queue, &presentInfo);
  if (result != VK_SUCCESS) {
    printf("present failed\n");
    exit(1);
  }
  e->currentFrame = (e->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void createModelBuffer(Engine *e) {
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingMemory;
  VkDeviceSize bufferSize = sizeof(Vertex) * e->modelVerticesNum;
  createBuffer(e, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               &stagingBuffer, &stagingMemory);

  void *data;
  vkMapMemory(e->device, stagingMemory, 0, bufferSize, 0, &data);
  memcpy(data, e->modelVertices, bufferSize);
  vkUnmapMemory(e->device, stagingMemory);
  createBuffer(e, bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &e->modelBuffer,
               &e->modelBufferMemory);
  copyBuffer(e, stagingBuffer, e->modelBuffer, bufferSize);
  vkDestroyBuffer(e->device, stagingBuffer, NULL);
  vkFreeMemory(e->device, stagingMemory, NULL);
}

// TODO mess end

Engine *makeEngine() {
  Engine *e = malloc(sizeof(Engine));
  e->msaaSample = VK_SAMPLE_COUNT_8_BIT;
  e->currentFrame = 0;
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
  createCommandPool(e);
  createColorResources(e);
  createDepthResources(e);
  createFramebuffers(e);
  createTextureImage(e);
  createTextureImageView(e);
  createTextureSampler(e);
  createUniformBuffers(e);
  createDescriptorPool(e);
  createDescriptorSets(e);
  createCommandBuffers(e);
  createSyncObjects(e);
  loadModel("assets/branch_t.obj", &e->modelVertices, &e->modelVerticesNum);
  createModelBuffer(e);
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

void destroyColorResources(Engine *e) {
  vkDestroyImageView(e->device, e->colorImageView, NULL);
  vkDestroyImage(e->device, e->colorImage, NULL);
  vkFreeMemory(e->device, e->colorImageMemory, NULL);
}

void destroyDepthResources(Engine *e) {
  vkDestroyImageView(e->device, e->depthImageView, NULL);
  vkDestroyImage(e->device, e->depthImage, NULL);
  vkFreeMemory(e->device, e->depthImageMemory, NULL);
}

void destroyFramebuffers(Engine *e) {
  for (uint32_t i = 0; i < e->imageCount; i++) {
    vkDestroyFramebuffer(e->device, e->framebuffers[i], NULL);
  }
  free(e->framebuffers);
}

void freeEngine(Engine *engine) {
  vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);
  destroySwapchainImages(engine);
  destroyColorResources(engine);
  destroyDepthResources(engine);
  destroyFramebuffers(engine);
  vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
  vkDestroyRenderPass(engine->device, engine->renderPass, NULL);
  vkDestroyPipelineLayout(engine->device, engine->pipelineLayout, NULL);
  vkDestroyPipeline(engine->device, engine->pipeline, NULL);
  vkDestroyDescriptorSetLayout(engine->device, engine->descriptorLayout, NULL);
  vkDestroyCommandPool(engine->device, engine->commandPool, NULL);
  vkDestroyDevice(engine->device, NULL);
  vkDestroyInstance(engine->instance, NULL);
  glfwTerminate();
  free(engine);
}

void run(Engine *e) {
  while (!glfwWindowShouldClose(e->window)) {
    glfwPollEvents();
    drawFrame(e);
  }
  vkDeviceWaitIdle(e->device);
}
