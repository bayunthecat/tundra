#ifndef VLK_RESOURCES
#define VLK_RESOURCES

// everything backed by a GPU memory

#include <vulkan/vulkan_core.h>

// TODO temporary place for a load
void* vlkLoad(const char* filepath, size_t* size);

void vlkCreateShaderModule(VkDevice device, const char* filepath,
                           VkShaderModule* module);

#endif  // !VLK_RESOURCES
