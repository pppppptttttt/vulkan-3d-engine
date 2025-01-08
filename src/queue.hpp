#pragma once

#include <cstdint> // for uint8_t
#include <limits>
#include <vulkan/vulkan_core.h> // for VkQueue, VK_NULL_HANDLE, VkDevice

namespace engine::core {

class CommandQueue {
private:
  VkQueue m_queue = VK_NULL_HANDLE;
  unsigned m_index = std::numeric_limits<unsigned>::max();

public:
  enum class Kind : std::uint8_t { GRAPHICS, PRESENT, TRANSFER };

  CommandQueue(VkPhysicalDevice physical_device, VkSurfaceKHR surface,
               VkDevice device, Kind kind);
  CommandQueue &submit(const VkSubmitInfo &submit_info, VkFence fence);
  CommandQueue &wait_idle();
  [[nodiscard]] unsigned index() const { return m_index; }
  [[nodiscard]] VkQueue queue() const { return m_queue; }
};

} // namespace engine::core
