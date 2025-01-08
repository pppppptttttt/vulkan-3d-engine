#pragma once

#include <optional>             // for optional
#include <vulkan/vulkan_core.h> // for VkPhysicalDevice, VkSurfaceKHR, VkIn...

namespace engine::core {
struct QueueFamilyIndices {
  std::optional<unsigned> graphics_family;
  std::optional<unsigned> present_family;
  std::optional<unsigned> transfer_family;
};

QueueFamilyIndices find_queue_families(VkPhysicalDevice device,
                                       VkSurfaceKHR surface);
VkPhysicalDevice choose_physical_device(VkInstance instance,
                                        VkSurfaceKHR surface);
unsigned find_memory_type(VkPhysicalDevice device, unsigned type_filter,
                          VkMemoryPropertyFlags properties);

} // namespace engine::core
