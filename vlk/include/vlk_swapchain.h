#ifndef VLK_SWAPCHAIN
#define VLK_SWAPCHAIN
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

void vlkCreateSurface(VkInstance instance, GLFWwindow* window,
                      VkSurfaceKHR* pSurface);

void vlkCreateSwapchainThin(VkDevice device, VkPhysicalDevice physicalDevice,
                            VkExtent2D extent, VkFormat format,
                            VkSurfaceKHR surface, VkSwapchainKHR* pSwapchain);

void vlkGetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain,
                           uint32_t* pImageCount, VkImage* pImages);

void vlkCreateSwapchainImageViews(VkDevice device, VkFormat format,
                                  uint32_t imageCount, VkImage* images,
                                  VkImageView* imageViews);

#endif  // !VLK_SWAPCHAIN
