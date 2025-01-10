#pragma once

#include "renderer.hpp"           // for Renderer
#include "shader.hpp"             // for Shader
#include "vulkan_destroyable.hpp" // for VkDestroyable, VkPipelineLayoutWra...
#include <cstddef>                // for size_t
#include <filesystem>             // for path
#include <map>                    // for map
#include <vulkan/vulkan_core.h>   // for VkShaderStageFlags, vkCmdPushConst...

namespace engine::resources {

class Material {
private:
  core::VkDestroyable<core::VkPipelineLayoutWrapper> m_pipeline_layout;
  core::VkDestroyable<core::VkPipelineWrapper> m_pipeline;
  void *m_push_constant_data = nullptr;
  std::size_t m_push_constant_size = 0;
  VkShaderStageFlags m_push_constant_stages = 0;

public:
  Material() = default;

  Material(core::Renderer &renderer,
           const std::map<core::Shader::Stage, std::filesystem::path> &shaders,
           void *push_constant_data = nullptr,
           std::size_t push_constant_size = 0,
           VkShaderStageFlags push_constant_stages = 0);

  [[nodiscard]] VkPipeline pipeline() const { return m_pipeline; }
  void update_push_constants(VkCommandBuffer command_buffer) const {
    if (m_push_constant_data) {
      vkCmdPushConstants(
          command_buffer, m_pipeline_layout, m_push_constant_stages, 0,
          static_cast<unsigned>(m_push_constant_size), m_push_constant_data);
    }
  }
};

} // namespace engine::resources
