#ifndef VLK_CONTEXT
#define VLK_CONTEXT

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

typedef struct VlkContext {
  VkInstance vkInstance;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkQueue queue;
  GLFWwindow* window;
} VlkContext;

void vlkCreateContex(VlkContext* context);

#endif  // !GPU_CONTEXT
