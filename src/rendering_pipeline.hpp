#pragma once

#include "shader.hpp"             // for Shader
#include "vulkan_destroyable.hpp" // for VkDestroyable, VkPipelineLayoutWra...
#include <filesystem>             // for path
#include <map>                    // for map
#include <vulkan/vulkan_core.h>   // for VkDevice, VkPipeline, VkRenderPass

namespace engine::core {

class Swapchain;

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
