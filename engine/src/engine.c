#include "engine.h"
#include "GLFW/glfw3.h"
#include "instance.h"
#include <cglm/affine-mat.h>
#include <cglm/affine.h>
#include <cglm/cglm.h>
#include <cglm/project.h>
#include <stdint.h>
#include <stdlib.h>
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

Engine *makeEngine() {
  Engine *e = malloc(sizeof(Engine));
  e->imageCount = 0;
  createInstance(&e->instance);
  // create surface
  // pick physical device
  // create logical device
  // get device queues
  // create swapchain
  // create image views
  // create render pass
  // cretate descriptor set layout
  // create graphics pipeline
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

void freeEngine(Engine *engine) {
  //
  free(engine);
}
