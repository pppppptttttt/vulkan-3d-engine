#pragma once

#include "vertex.hpp"         // for Vertex
#include "vulkan_buffers.hpp" // for Buffer, Renderer
#include <span>               // for span

namespace engine::resources {

class Mesh {
private:
  core::Buffer m_vertices;

public:
  Mesh() = default;
  Mesh(core::Renderer &renderer, std::span<Vertex> vertices);
  [[nodiscard]] const core::Buffer &vertices() const { return m_vertices; }
};

} // namespace engine::resources
