#ifndef VLK_CONTEXT
#define VLK_CONTEXT

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

void vlkCreateInstance(VkInstance* pInstance);

void vlkCreateDevice(VkInstance instance, VkPhysicalDevice* pPhysicalDevice,
                     VkDevice* pLogicalDevice);

#endif  // !GPU_CONTEXT
