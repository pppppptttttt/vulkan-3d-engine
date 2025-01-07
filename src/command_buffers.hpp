#pragma once

#include "engine_exceptions.hpp"  // for CommandPoolRecordError
#include "vulkan_destroyable.hpp" // for VkCommandPoolWrapper, VkDestroyable
#include <concepts>               // for invocable
#include <cstddef>                // for size_t
#include <vector>                 // for vector
#include <vulkan/vulkan_core.h>   // for VkCommandBuffer, VkDevice, VkResult

namespace engine::core {

class CommandBuffer {
private:
  VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;

public:
  CommandBuffer() = default;
  CommandBuffer(VkCommandBuffer buffer) : m_command_buffer(buffer) {}

  void reset(VkCommandBufferResetFlags flags = 0) {
    vkResetCommandBuffer(m_command_buffer, flags);
  }

  void record(const std::invocable<VkCommandBuffer> auto &function) {
    const VkCommandBufferBeginInfo command_buffer_begin{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr};

    if (vkBeginCommandBuffer(m_command_buffer, &command_buffer_begin) !=
        VK_SUCCESS) {
      throw exceptions::CommandPoolRecordError{};
    }

    function(m_command_buffer);

    if (vkEndCommandBuffer(m_command_buffer) != VK_SUCCESS) {
      throw exceptions::CommandPoolRecordError{};
    }
  }

  [[nodiscard]] VkCommandBuffer buffer() const { return m_command_buffer; }
};

class CommandPool {
private:
  VkDestroyable<VkCommandPoolWrapper> m_command_pool;
  VkDevice m_device;

public:
  CommandPool(VkDevice device, VkPhysicalDevice physical_device,
              VkSurfaceKHR surface);

  [[nodiscard]] std::vector<CommandBuffer>
  make_command_buffers(std::size_t count) const;
};

} // namespace engine::core
