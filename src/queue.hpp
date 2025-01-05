#pragma once

#include <cstdint>              // for uint8_t
#include <vulkan/vulkan_core.h> // for VkQueue, VK_NULL_HANDLE, VkDevice

namespace engine::core {

class CommandQueue {
private:
  VkQueue m_queue = VK_NULL_HANDLE;

public:
  enum class Kind : std::uint8_t { GRAPHICS, PRESENT, TRANSFER };

  CommandQueue(VkPhysicalDevice physical_device, VkSurfaceKHR surface,
               VkDevice device, Kind kind);
  void submit(const VkSubmitInfo &submit_info, VkFence fence) const;
  [[nodiscard]] VkQueue queue() const { return m_queue; }
};

} // namespace engine::core
