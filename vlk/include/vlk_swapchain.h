#ifndef VLK_SWAPCHAIN
#define VLK_SWAPCHAIN
#define VLK_SWAPCHAIN_IMAGE_SLOTS 10

#include <vulkan/vulkan_core.h>

#include "vlk_context.h"

typedef struct VlkSwapchain {
  uint32_t swapchainImageCount;
  VkFormat swapchainImageFormat;
  VkExtent2D swapchainExtent;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  VkImage swapchainImages[VLK_SWAPCHAIN_IMAGE_SLOTS];
  VkImageView swapchainImageViews[VLK_SWAPCHAIN_IMAGE_SLOTS];
} VlkSwapchain;

void vlkCreateSwapchain(VlkContext* vlkContext, VlkSwapchain* vlkSwapchain);

void vlkDestroySwapchain(VlkContext* vlkContext, VlkSwapchain* vlkSwapchain);

#endif  // !VLK_SWAPCHAIN
