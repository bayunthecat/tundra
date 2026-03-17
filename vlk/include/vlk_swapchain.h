#ifndef VLK_SWAPCHAIN
#define VLK_SWAPCHAIN

#include <vulkan/vulkan_core.h>

#include "vlk_context.h"

typedef struct VlkSwapchain {
  uint32_t swapchainImageCount;
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  VkImage swapchainImages[10];
  VkImageView swapchainImageViews[10];
} VlkSwapchain;

void vlkCreateSwapchain(VlkContext* context, VlkSwapchain* vlkSwapchain);

#endif  // !VLK_SWAPCHAIN
