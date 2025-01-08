#include "vulkan_buffers.hpp"
#include "command_buffers.hpp"         // for CommandBuffer, CommandPool
#include "engine_exceptions.hpp"       // for BufferCreationError, MemoryAl...
#include "physical_device_queries.hpp" // for find_memory_type
#include "queue.hpp"                   // for CommandQueue
#include "renderer.hpp"                // for Renderer
#include <cstddef>                     // for byte
#include <cstring>                     // for memcpy, size_t
#include <vector>                      // for vector
#include <vulkan/vulkan_core.h>        // for VkStructureType, VK_NULL_HANDLE

namespace engine::core {

Buffer::Buffer(Renderer &renderer, VkDeviceSize size, VkBufferUsageFlags usage,
               VkSharingMode sharing_mode, unsigned queue_family_index_count,
               const unsigned *queue_family_indices)
    : m_size(size), m_renderer(&renderer) {
  const VkBufferCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage = usage,
      .sharingMode = sharing_mode,
      .queueFamilyIndexCount = queue_family_index_count,
      .pQueueFamilyIndices = queue_family_indices};

  VkBuffer buffer = VK_NULL_HANDLE;
  if (vkCreateBuffer(renderer.device(), &create_info, nullptr, &buffer) !=
      VK_SUCCESS) {
    throw exceptions::BufferCreationError{};
  }
  m_buffer = {buffer, renderer.device()};
}

Buffer &Buffer::allocate(unsigned mem_properties) {
  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(m_renderer->device(), m_buffer,
                                &mem_requirements);

  const VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = mem_requirements.size,
      .memoryTypeIndex =
          find_memory_type(m_renderer->physical_device(),
                           mem_requirements.memoryTypeBits, mem_properties),
  };

  VkDeviceMemory memory = VK_NULL_HANDLE;
  if (vkAllocateMemory(m_renderer->device(), &allocInfo, nullptr, &memory) !=
      VK_SUCCESS) {
    throw exceptions::MemoryAllocationError{};
  }
  m_memory = {memory, m_renderer->device()};

  vkBindBufferMemory(m_renderer->device(), m_buffer, m_memory, 0);

  return *this;
}

void Buffer::upload(const std::byte *data) {
  void *mapped = nullptr;
  vkMapMemory(m_renderer->device(), m_memory, 0, m_size, 0, &mapped);
  std::memcpy(mapped, data, static_cast<std::size_t>(m_size));
  vkUnmapMemory(m_renderer->device(), m_memory);
}

Buffer::Buffer(const Buffer &other) { *this = other; }

Buffer &Buffer::operator=(const Buffer &other) {
  if (this == &other) {
    return *this;
  }
  CommandBuffer command_buffer =
      m_renderer->transfer_command_pool().make_command_buffers(1).front();

  command_buffer.record(
      [this, &other](VkCommandBuffer command_buffer) {
        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0; // Optional
        copy_region.dstOffset = 0; // Optional
        copy_region.size = m_size;
        vkCmdCopyBuffer(command_buffer, other.m_buffer, m_buffer, 1,
                        &copy_region);
      },
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  VkCommandBuffer buf = command_buffer.buffer();
  submit_info.pCommandBuffers = &buf;

  m_renderer->queue(CommandQueue::Kind::TRANSFER)
      .submit(submit_info, VK_NULL_HANDLE)
      .wait_idle();

  m_renderer->transfer_command_pool().free_command_buffers({command_buffer});

  return *this;
}

} // namespace engine::core
