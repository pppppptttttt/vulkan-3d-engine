#pragma once

#include "vulkan_destroyable.hpp" // for VkDestroyable, VkFenceWrapper, VkS...
#include <cstdint>                // for uint64_t
#include <limits>                 // for numeric_limits
#include <vulkan/vulkan_core.h>   // for VkDevice, vkResetFences, vkWaitFor...

namespace engine::core {

class Fence {
private:
  VkDestroyable<VkFenceWrapper> m_fence;
  VkDevice m_device = VK_NULL_HANDLE;

public:
  Fence() = default;

  Fence(VkDevice device);

  void wait() {
    vkWaitForFences(m_device, 1, &m_fence, VK_TRUE,
                    std::numeric_limits<std::uint64_t>::max());
  }

  void reset() { vkResetFences(m_device, 1, &m_fence); }

  [[nodiscard]] VkFence fence() const { return m_fence; }
};

class Semaphore {
private:
  VkDestroyable<VkSemaphoreWrapper> m_semaphore;

public:
  Semaphore() = default;

  Semaphore(VkDevice device);

  [[nodiscard]] VkSemaphore semaphore() const { return m_semaphore; }
};

} // namespace engine::core
