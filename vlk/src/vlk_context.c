#include "vlk_context.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

static void createGlfw(GLFWwindow** window, int width, int height) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  // TODO change Hello vulkan
  *window = glfwCreateWindow(width, height, "Hello Vulkan", NULL, NULL);
}

static void createLogicalDevice(VkPhysicalDevice* physicalDevice,
                                VkDevice* device) {
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
  const char** ext = (const char*[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkDeviceCreateInfo deviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .pEnabledFeatures = &features,
      .enabledExtensionCount = 1,
      .ppEnabledExtensionNames = ext};
  VkResult result =
      vkCreateDevice(*physicalDevice, &deviceCreateInfo, NULL, device);
  if (result != VK_SUCCESS) {
    printf("failed to create logical device, error code: %d\n", result);
  }
}

static void pickPhysicalDevice(VkInstance* instance,
                               VkPhysicalDevice* physicalDevice) {
  uint32_t deviceCount;
  vkEnumeratePhysicalDevices(*instance, &deviceCount, NULL);
  if (deviceCount < 1) {
    printf("no compatible devices present\n");
    exit(1);
  }
  VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(*instance, &deviceCount, devices);
  // skipped queue introspection logic for simplicity
  *physicalDevice = devices[0];
  free(devices);
  printf("selected physical device: %p\n", *physicalDevice);
}

static void createInstance(VkInstance* instance) {
  uint32_t extCount = 0;
  const char** extensions = glfwGetRequiredInstanceExtensions(&extCount);
  uint32_t layersCount = 1;
  const char** layers = (const char*[]){"VK_LAYER_KHRONOS_validation"};
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
  printf("instance created: %p\n", instance);
}

void vlkCreateContex(VlkContext* context) {
  // TODO not a place in a context, as it is a platform dependent code
  createGlfw(&context->window, 800, 600);
  createInstance(&context->vkInstance);
  pickPhysicalDevice(&context->vkInstance, &context->physicalDevice);
  createLogicalDevice(&context->physicalDevice, &context->device);
  vkGetDeviceQueue(context->device, 0, 0, &context->queue);
}
