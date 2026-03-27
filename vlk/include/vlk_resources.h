#ifndef VLK_RESOURCES
#define VLK_RESOURCES

// everything backed by a GPU memory

#include <vulkan/vulkan_core.h>

// TODO temporary place for a load
void* vlkLoad(const char* filepath, size_t* size);

uint32_t vlkFindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                           VkMemoryPropertyFlags props);

// TODO think about further decomposition
void vlkCreateImage(VkDevice device, VkPhysicalDevice physicalDevice,
                    uint32_t width, uint32_t height, uint32_t mipLevels,
                    VkSampleCountFlagBits numSamples, VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage,
                    VkMemoryPropertyFlags props, VkImage* image,
                    VkDeviceMemory* imageMemory);

void vlkCreateImageView(VkDevice device, VkImage image, VkFormat format,
                        VkImageAspectFlags aspectFlags, uint32_t mipLevels,
                        VkImageView* imageView);

void vlkCreateShaderModule(VkDevice device, const char* filepath,
                           VkShaderModule* module);

void vlkCreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                     VkDeviceSize size, VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags props, VkBuffer* buffer,
                     VkDeviceMemory* memory);

#endif  // !VLK_RESOURCES
