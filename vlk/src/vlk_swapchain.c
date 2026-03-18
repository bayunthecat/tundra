#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "vlk_swapchain.h"

static VkImageView createImageView(VkDevice device, VkImage image,
                                   VkFormat format,
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
  if (vkCreateImageView(device, &info, NULL, &imageView) != VK_SUCCESS) {
    printf("failed to create image view\n");
    exit(1);
  }
  return imageView;
}

static void destroyImageViews(VkDevice device, VkImageView* imageViews,
                              uint32_t imageCount) {
  for (uint32_t i = 0; i < imageCount; i++) {
    vkDestroyImageView(device, imageViews[i], NULL);
  }
}

void vlkCreateSurface(VkInstance instance, GLFWwindow* window,
                      VkSurfaceKHR* pSurface) {
  printf("creating surface\n");
  VkResult result = glfwCreateWindowSurface(instance, window, NULL, pSurface);
  if (result != VK_SUCCESS) {
    printf("failed to create window surface, error code: %d\n", result);
    exit(1);
  }
}

void vlkCreateSwapchainThin(VkDevice device, VkPhysicalDevice physicalDevice,
                            VkExtent2D extent, VkFormat format,
                            VkSurfaceKHR surface, VkSwapchainKHR* pSwapchain) {
  printf("creating swapchain\n");
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
  uint32_t swapchainImageCount = caps.minImageCount + 1;
  VkSwapchainCreateInfoKHR info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .imageFormat = format,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
      .imageExtent = extent,
      .surface = surface,
      .minImageCount = swapchainImageCount,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = 0,
      .preTransform = caps.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };
  VkResult result = vkCreateSwapchainKHR(device, &info, NULL, pSwapchain);
  if (result != VK_SUCCESS) {
    printf("failed to create swapchain\n");
    exit(1);
  }
}

void vlkGetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain,
                           uint32_t* pImageCount, VkImage* pImages) {
  vkGetSwapchainImagesKHR(device, swapchain, pImageCount, NULL);
  if (pImageCount == 0) {
    printf("no images in the swapchain\n");
    exit(1);
  }
  printf("image count: %d\n", *pImageCount);
  vkGetSwapchainImagesKHR(device, swapchain, pImageCount, pImages);
}

void vlkCreateSwapchainImageViews(VkDevice device, VkFormat format,
                                  uint32_t imageViewCount, VkImage* images,
                                  VkImageView* imageViews) {
  printf("creating swapchain image views\n");
  for (uint32_t i = 0; i < imageViewCount; i++) {
    imageViews[i] = createImageView(device, images[i], format,
                                    VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}
