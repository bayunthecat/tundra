#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "vlk_context.h"
#include "vlk_swapchain.h"

static void createSurface(VlkContext* vlkContext, VlkSwapchain* vlkSwapchain) {
  printf("creating surface\n");
  VkResult result = glfwCreateWindowSurface(
      vlkContext->vkInstance, vlkContext->window, NULL, &vlkSwapchain->surface);
  if (result != VK_SUCCESS) {
    printf("failed to create window surface, error code: %d\n", result);
    exit(1);
  }
}

static void createSwapchain(VlkContext* vlkContext,
                            VlkSwapchain* vlkSwapchain) {
  printf("creating swapchain\n");
  VkSurfaceCapabilitiesKHR surfaceCaps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      vlkContext->physicalDevice, vlkSwapchain->surface, &surfaceCaps);
  uint32_t swapchainImageCount = surfaceCaps.minImageCount + 1;
  vlkSwapchain->swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  VkSwapchainCreateInfoKHR info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .imageFormat = vlkSwapchain->swapchainImageFormat,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
      .imageExtent = vlkSwapchain->swapchainExtent,
      .surface = vlkSwapchain->surface,
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
  VkResult result = vkCreateSwapchainKHR(vlkContext->device, &info, NULL,
                                         &vlkSwapchain->swapchain);
  if (result != VK_SUCCESS) {
    printf("failed to create swapchain\n");
    exit(1);
  }
  vkGetSwapchainImagesKHR(vlkContext->device, vlkSwapchain->swapchain,
                          &vlkSwapchain->swapchainImageCount, NULL);
  if (vlkSwapchain->swapchainImageCount == 0) {
    printf("no images in the swapchain\n");
    exit(1);
  } else if (vlkSwapchain->swapchainImageCount > 10) {
    printf("number of swapchain images exceeds available memory slots.");
    exit(1);
  }
  vkGetSwapchainImagesKHR(vlkContext->device, vlkSwapchain->swapchain,
                          &vlkSwapchain->swapchainImageCount,
                          vlkSwapchain->swapchainImages);
}

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

static void createSwapchainImageViews(VlkContext* vlkContext,
                                      VlkSwapchain* vlkSwapchain) {
  printf("creating swapchain image views\n");
  for (uint32_t i = 0; i < vlkSwapchain->swapchainImageCount; i++) {
    vlkSwapchain->swapchainImageViews[i] = createImageView(
        vlkContext->device, vlkSwapchain->swapchainImages[i],
        vlkSwapchain->swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void vlkCreateSwapchain(VlkContext* vlkContext, VlkSwapchain* vlkSwapchain) {
  // TODO hardcoded swpchain extent dimensions
  vlkSwapchain->swapchainExtent = (VkExtent2D){
      .width = 800,
      .height = 600,
  };
  createSurface(vlkContext, vlkSwapchain);
  createSwapchain(vlkContext, vlkSwapchain);
  createSwapchainImageViews(vlkContext, vlkSwapchain);
}
