#pragma once

#include <optional>
#include <vulkan/vulkan.h>

namespace engine::core {
struct QueueFamilyIndices {
  std::optional<unsigned> graphics_family;
  std::optional<unsigned> present_family;
  std::optional<unsigned> transfer_family;
};

QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
VkPhysicalDevice choose_physical_device(VkInstance instance, VkSurfaceKHR surface);

} // namespace engine::core
