#pragma once

#include "engine_exceptions.hpp"
#include "vulkan_destroyable.hpp"
#include <vulkan/vulkan_core.h>

namespace engine::core {

class Fence {
private:
  VkDestroyable<VkFenceWrapper> m_fence;
  VkDevice m_device;

public:
  Fence() = default;

  Fence(VkDevice device) : m_device(device) {
    const VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    VkFence fence = VK_NULL_HANDLE;
    if (vkCreateFence(device, &fence_create_info, nullptr, &fence) !=
        VK_SUCCESS) {
      throw exceptions::SyncPrimitivesCreationError{};
    }
    m_fence = {fence, device};
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
  Semaphore() = default;

  Semaphore(VkDevice device) {
    const VkSemaphoreCreateInfo sem_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0};

    VkSemaphore semaphore = VK_NULL_HANDLE;
    if (vkCreateSemaphore(device, &sem_create_info, nullptr, &semaphore) !=
        VK_SUCCESS) {
      throw exceptions::SyncPrimitivesCreationError{};
    }
    m_semaphore = {semaphore, device};
  }

  [[nodiscard]] VkSemaphore semaphore() const { return m_semaphore; }
};

} // namespace engine::core
