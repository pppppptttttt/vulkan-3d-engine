#pragma once

#include "engine_exceptions.hpp"
#include "vulkan_destroyable.hpp"

namespace engine::core {

class Fence {
private:
  VkDestroyable<VkFenceWrapper> m_fence;
  VkDevice m_device;

public:
  Fence(VkDevice device) : m_device(device) {
    const VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    if (vkCreateFence(device, &fence_create_info, nullptr, &m_fence) !=
        VK_SUCCESS) {
      throw exceptions::SyncPrimitivesCreationError{};
    }
    m_fence.wrapped.parent = device;
    m_fence.wrapped.destroy_function = vkDestroyFence;
  }

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
  Semaphore(VkDevice device) {
    const VkSemaphoreCreateInfo sem_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0};

    if (vkCreateSemaphore(device, &sem_create_info, nullptr, &m_semaphore) !=
        VK_SUCCESS) {
      throw exceptions::SyncPrimitivesCreationError{};
    }
    m_semaphore.wrapped.parent = device;
    m_semaphore.wrapped.destroy_function = vkDestroySemaphore;
  }

  [[nodiscard]] VkSemaphore semaphore() const { return m_semaphore; }
};

} // namespace engine::core
