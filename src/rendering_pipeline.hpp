#pragma once

#include "shader.hpp"
#include "swapchain.hpp"
#include <filesystem>
#include <map>

namespace engine::core {

class RenderingPipeline {
private:
  VkDestroyable<VkRenderPassWrapper> m_render_pass;
  VkDestroyable<VkPipelineLayoutWrapper> m_pipeline_layout;
  VkDestroyable<VkPipelineWrapper> m_pipeline;

public:
  RenderingPipeline(
      VkDevice device, Swapchain &swapchain,
      const std::map<Shader::Stage, std::filesystem::path> &shaders);

  [[nodiscard]] VkPipeline pipeline() const { return m_pipeline; }
  [[nodiscard]] VkRenderPass render_pass() const { return m_render_pass; }
};

} // namespace engine::core
