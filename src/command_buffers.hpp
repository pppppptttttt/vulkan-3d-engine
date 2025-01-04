#pragma once

#include "engine_exceptions.hpp"
#include "physical_device_queries.hpp"
#include "vulkan_destroyable.hpp"
#include <cassert>
#include <concepts>
#include <vector>
#include <vulkan/vulkan_core.h>

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
              VkSurfaceKHR surface)
      : m_device(device) {
    QueueFamilyIndices ind = find_queue_families(physical_device, surface);
    assert(ind.graphics_family);

    const VkCommandPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = *ind.graphics_family};

    if (vkCreateCommandPool(device, &create_info, nullptr, &m_command_pool) !=
        VK_SUCCESS) {
      throw exceptions::CommandPoolCreationError{};
    }
    m_command_pool.wrapped.parent = device;
    m_command_pool.wrapped.destroy_function = vkDestroyCommandPool;
  }

  [[nodiscard]] std::vector<CommandBuffer> make_command_buffers(std::size_t count) const {
    std::vector<VkCommandBuffer> vk_command_buffers(count);
    const VkCommandBufferAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<unsigned>(vk_command_buffers.size())};

    if (vkAllocateCommandBuffers(m_device, &allocate_info,
                                 vk_command_buffers.data()) != VK_SUCCESS) {
      throw exceptions::CommandPoolAllocaionError{};
    }

    std::vector<CommandBuffer> command_buffers(count);
    for (std::size_t i = 0; i < count; ++i) {
      command_buffers[i] = vk_command_buffers[i];
    }
    return command_buffers;
  }
};

} // namespace engine::core
