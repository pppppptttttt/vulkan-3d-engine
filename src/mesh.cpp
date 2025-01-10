#include "mesh.hpp"
#include "material.hpp"
#include "queue.hpp"            // for CommandQueue, CommandQueue::Kind::GR...
#include "renderer.hpp"         // for Renderer
#include "vulkan_buffers.hpp"   // for Buffer
#include <cstddef>              // for byte
#include <set>                  // for set, operator==
#include <vector>               // for vector
#include <vulkan/vulkan_core.h> // for VkBufferUsageFlagBits, VkMemoryPrope...

namespace engine::resources {

Mesh::Mesh(core::Renderer &renderer, std::span<Vertex> vertices,
           std::span<unsigned> indices, const resources::Material *material)
    : m_vertices(renderer, vertices.size() * sizeof(vertices.front()),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
      m_indices(renderer, indices.size() * sizeof(indices.front()),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
      m_indices_size(indices.size()), m_material(material) {
  using enum core::CommandQueue::Kind;
  const std::set<unsigned> unique_queue_indices = {
      renderer.queue(TRANSFER).index(), renderer.queue(GRAPHICS).index()};
  const std::vector<unsigned> queue_indices(unique_queue_indices.begin(),
                                            unique_queue_indices.end());

  core::Buffer staging(renderer, vertices.size() * sizeof(vertices.front()),
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       queue_indices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE
                                                 : VK_SHARING_MODE_CONCURRENT,
                       static_cast<unsigned>(queue_indices.size()),
                       queue_indices.data());
  staging.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  m_vertices.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  staging.upload(reinterpret_cast<std::byte *>(vertices.data())); // NOLINT
  m_vertices = staging;

  staging = core::Buffer(renderer, indices.size() * sizeof(indices.front()),
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         queue_indices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE
                                                   : VK_SHARING_MODE_CONCURRENT,
                         static_cast<unsigned>(queue_indices.size()),
                         queue_indices.data());
  staging.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  m_indices.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  staging.upload(reinterpret_cast<std::byte *>(indices.data())); // NOLINT
  m_indices = staging;
}

[[nodiscard]] VkPipeline Mesh::pipeline() const {
  return m_material->pipeline();
}

} // namespace engine::resources
