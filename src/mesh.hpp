#pragma once

#include "vertex.hpp"           // for Vertex
#include "vulkan_buffers.hpp"   // for Buffer, Renderer
#include <cstddef>              // for size_t
#include <span>                 // for span
#include <vulkan/vulkan_core.h> // for VkPipeline

namespace engine::resources {

class Material;

class Mesh {
private:
  core::Buffer m_vertices;
  core::Buffer m_indices;
  unsigned m_indices_size = 0;
  const resources::Material *m_material = nullptr;

public:
  Mesh() = default;
  Mesh(core::Renderer &renderer, std::span<Vertex> vertices,
       std::span<unsigned> indices, const resources::Material *material);

  [[nodiscard]] std::size_t indices_size() const { return m_indices_size; }
  [[nodiscard]] const core::Buffer &vertices() const { return m_vertices; }
  [[nodiscard]] const core::Buffer &indices() const { return m_indices; }
  [[nodiscard]] VkPipeline pipeline() const;
  [[nodiscard]] const Material *material() const { return m_material; }
};

} // namespace engine::resources
