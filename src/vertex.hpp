#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <cstddef>
#include <vulkan/vulkan_core.h>

namespace engine::resources {

struct Vertex {
  glm::vec3 position; // NOLINT
  glm::vec2 uv;       // NOLINT
  glm::vec3 normal;   // NOLINT
  glm::vec4 color;    // NOLINT

  static VkVertexInputBindingDescription binding_description() noexcept {
    return {.binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
  }

  static std::array<VkVertexInputAttributeDescription, 4>
  attribute_description() noexcept {
    return std::array<VkVertexInputAttributeDescription, 4>{
        {{
             .location = 0,
             .binding = 0,
             .format = VK_FORMAT_R32G32B32_SFLOAT,
             .offset = offsetof(Vertex, position),
         },
         {
             .location = 1,
             .binding = 0,
             .format = VK_FORMAT_R32G32_SFLOAT,
             .offset = offsetof(Vertex, uv),
         },
         {
             .location = 2,
             .binding = 0,
             .format = VK_FORMAT_R32G32B32_SFLOAT,
             .offset = offsetof(Vertex, normal),
         },
         {
             .location = 3,
             .binding = 0,
             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
             .offset = offsetof(Vertex, color),
         }}};
  }
};

} // namespace engine::resources
