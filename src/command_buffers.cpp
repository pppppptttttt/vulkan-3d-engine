#include "command_buffers.hpp"
#include "physical_device_queries.hpp" // for QueueFamilyIndices, find_queu...
#include <cassert>                     // for assert
#include <optional>                    // for optional

namespace engine::core {

CommandPool::CommandPool(VkDevice device, VkPhysicalDevice physical_device,
                         VkSurfaceKHR surface)
    : m_device(device) {
  QueueFamilyIndices ind = find_queue_families(physical_device, surface);
  assert(ind.graphics_family);

  const VkCommandPoolCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = *ind.graphics_family};

  VkCommandPool command_pool = VK_NULL_HANDLE;
  if (vkCreateCommandPool(device, &create_info, nullptr, &command_pool) !=
      VK_SUCCESS) {
    throw exceptions::CommandPoolCreationError{};
  }
  m_command_pool = {command_pool, device};
}

[[nodiscard]] std::vector<CommandBuffer>
CommandPool::make_command_buffers(std::size_t count) const {
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

} // namespace engine::core
