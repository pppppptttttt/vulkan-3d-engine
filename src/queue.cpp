#include "queue.hpp"
#include "engine_exceptions.hpp"       // for SubmitCommandBufferError
#include "physical_device_queries.hpp" // for QueueFamilyIndices, find_queu...
#include <cassert>                     // for assert
#include <optional>                    // for optional
#include <utility>                     // for unreachable
#include <vulkan/vulkan_core.h>

namespace engine::core {

CommandQueue::CommandQueue(VkPhysicalDevice physical_device,
                           VkSurfaceKHR surface, VkDevice device, Kind kind) {
  const auto ind = find_queue_families(physical_device, surface);
  assert(ind.graphics_family && ind.transfer_family && ind.present_family);
  switch (kind) {
  case Kind::GRAPHICS:
    assert(ind.graphics_family);
    vkGetDeviceQueue(device, *ind.graphics_family, 0, &m_queue);
    break;
  case Kind::PRESENT:
    assert(ind.graphics_family);
    vkGetDeviceQueue(device, *ind.present_family, 0, &m_queue);
    break;
  case Kind::TRANSFER:
    assert(ind.graphics_family);
    vkGetDeviceQueue(device, *ind.transfer_family, 0, &m_queue);
    break;
  default:
    std::unreachable();
  }
}

CommandQueue &CommandQueue::submit(const VkSubmitInfo &submit_info,
                                   VkFence fence) {
  if (vkQueueSubmit(m_queue, 1, &submit_info, fence) != VK_SUCCESS) {
    throw exceptions::SubmitCommandBufferError{};
  }
  return *this;
}

CommandQueue &CommandQueue::wait_idle() {
  vkQueueWaitIdle(m_queue);
  return *this;
}

} // namespace engine::core
