#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#include "vlk_context.h"
#include "vlk_pipeline.h"
#include "vlk_swapchain.h"

#define SW_SLOTS 10

typedef struct {
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkSurfaceKHR surface;
  VkFormat format;
  VkExtent2D extent;
  VkSwapchainKHR swapchain;
  uint32_t imageCount;
  VkImage swapchainImages[SW_SLOTS];
  VkImageView swapchainImageViews[SW_SLOTS];
  VkRenderPass renderPass;
} Vlk;

void vlkInit(Vlk* vlk, GLFWwindow* window) {
  vlkCreateInstance(&vlk->instance);
  vlkCreateDevice(vlk->instance, &vlk->physicalDevice, &vlk->device);
  vlkCreateSurface(vlk->instance, window, &vlk->surface);
  vlkCreateSwapchain(vlk->device, vlk->physicalDevice, vlk->extent, vlk->format,
                     vlk->surface, &vlk->swapchain);
  vlkGetSwapchainImages(vlk->device, vlk->swapchain, &vlk->imageCount,
                        vlk->swapchainImages);
  vlkCreateSwapchainImageViews(vlk->device, vlk->format, vlk->imageCount,
                               vlk->swapchainImages, vlk->swapchainImageViews);
  vlkCreateRenderPass(vlk->device, vlk->format, 0, &vlk->renderPass);
}

void vlkDestroy(Vlk* vlk) {
  for (int i = 0; i < vlk->imageCount; i++) {
    vkDestroyImageView(vlk->device, vlk->swapchainImageViews[i], NULL);
  }
  vkDestroySwapchainKHR(vlk->device, vlk->swapchain, NULL);
  vkDestroySurfaceKHR(vlk->instance, vlk->surface, NULL);
  vkDestroyDevice(vlk->device, NULL);
  vkDestroyInstance(vlk->instance, NULL);
}

int main() {
  Vlk vlk;
  vlk.extent = (VkExtent2D){800, 800};
  vlk.format = VK_FORMAT_B8G8R8A8_UNORM;
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(vlk.extent.width, vlk.extent.height,
                                        "sandbox", NULL, NULL);
  vlkInit(&vlk, window);
  vlkDestroy(&vlk);
  return 0;
}
