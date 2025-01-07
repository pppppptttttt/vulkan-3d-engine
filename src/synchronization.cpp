#include "synchronization.hpp"
#include "engine_exceptions.hpp" // for SyncPrimitivesCreationError

namespace engine::core {

Fence::Fence(VkDevice device) : m_device(device) {
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

Semaphore::Semaphore(VkDevice device) {
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

} // namespace engine::core
