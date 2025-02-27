#include "physical_device_queries.hpp"
#include "engine_exceptions.hpp" // for SuitableGPUNotFound
#include "meta.hpp"              // for DEVICE_EXTENSIONS
#include <algorithm>             // for find_if, remove
#include <array>                 // for array
#include <cstddef>               // for size_t
#include <map>                   // for map, operator==, _Rb_tree_iterator
#include <string>                // for operator==, string
#include <utility>               // for pair
#include <vector>                // for vector

namespace {

engine::core::QueueFamilyIndices
find_graphics_present_queue_families(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
  engine::core::QueueFamilyIndices ind;

  unsigned count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

  for (std::size_t i = 0; i < families.size(); ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      ind.graphics_family = i;
    }

    if (families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      ind.transfer_family = i;
    }

    VkBool32 present = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);
    if (present) {
      ind.present_family = i;
    }

    if (ind.graphics_family && ind.present_family) {
      return ind;
    }
  }
  return ind;
}

bool is_device_extensions_supported(VkPhysicalDevice device) {
  unsigned count = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> extensions(count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count,
                                       extensions.data());
  std::vector<std::string> required(engine::core::DEVICE_EXTENSIONS.begin(),
                                    engine::core::DEVICE_EXTENSIONS.end());
  for (const auto &e : extensions) {
    required.erase(
        std::remove(required.begin(), required.end(), e.extensionName),
        required.end());
  }
  return required.empty();
}

} // namespace

namespace engine::core {

QueueFamilyIndices find_queue_families(VkPhysicalDevice device,
                                       VkSurfaceKHR surface) {
  static std::map<std::pair<VkPhysicalDevice, VkSurfaceKHR>, QueueFamilyIndices>
      cache;
  if (auto it = cache.find({device, surface}); it != cache.end()) {
    return it->second;
  }

  auto ind = find_graphics_present_queue_families(device, surface);

  unsigned count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

  // search in separate queue
  for (std::size_t i = 0; i < families.size(); ++i) {
    if ((families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
        !(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
      ind.transfer_family = i;
      return ind;
    }
  }

  // graphics queue supports implicitly supports transfer operations
  ind.transfer_family = ind.graphics_family;
  return cache[{device, surface}] = ind;
}

VkPhysicalDevice choose_physical_device(VkInstance instance,
                                        VkSurfaceKHR surface) {
  unsigned device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
  if (device_count == 0) {
    throw exceptions::SuitableGPUNotFound{};
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  auto suitable = [surface](VkPhysicalDevice device) -> bool {
    const auto ind = find_queue_families(device, surface);
    const bool extensions_support = is_device_extensions_supported(device);
    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(device, &supported_features);
    return ind.present_family && ind.graphics_family && extensions_support &&
           supported_features.samplerAnisotropy;
  };

  const auto device_it = std::find_if(devices.begin(), devices.end(), suitable);
  if (device_it == devices.end()) {
    throw exceptions::SuitableGPUNotFound{};
  }
  return *device_it;
}

unsigned find_memory_type(VkPhysicalDevice device, unsigned type_filter,
                          VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties{};
  vkGetPhysicalDeviceMemoryProperties(device, &mem_properties);

  for (unsigned i = 0; i < mem_properties.memoryTypeCount; i++) {
    if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags &
                                   properties) == properties) {
      return i;
    }
  }

  throw exceptions::SuitableMemoryTypeNotFound{};
  return 0;
}

} // namespace engine::core
