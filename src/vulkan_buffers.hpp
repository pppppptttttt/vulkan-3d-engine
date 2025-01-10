#pragma once

#include "vulkan_destroyable.hpp" // for VkDestroyable, VkBufferWrapper
#include <cstddef>                // for byte
#include <vulkan/vulkan_core.h>   // for VkSharingMode, VkDeviceSize, VkBuffer

namespace engine::core {

class Renderer;

class Buffer {
private:
  VkDestroyable<VkBufferWrapper> m_buffer;
  VkDeviceSize m_size = 0;
  VkDestroyable<VkDeviceMemoryWrapper> m_memory;
  Renderer *m_renderer = nullptr;

public:
  Buffer() = default;

  [[nodiscard]] VkBuffer buffer() const { return m_buffer; }

  Buffer(Renderer &renderer, VkDeviceSize size, VkBufferUsageFlags usage,
         VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
         unsigned queue_family_index_count = 0,
         const unsigned *queue_family_indices = nullptr);

  Buffer &allocate(unsigned mem_properties = 0);

  void upload(const std::byte *data);

  Buffer(const Buffer &other);
  Buffer &operator=(const Buffer &other);

  Buffer(Buffer &&) = default;
  Buffer &operator=(Buffer &&) = default;
  ~Buffer() = default;
};

} // namespace engine::core
