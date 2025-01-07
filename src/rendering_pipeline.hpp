#pragma once

#include "shader.hpp"             // for Shader
#include "vulkan_destroyable.hpp" // for VkDestroyable, VkPipelineLayoutWra...
#include <filesystem>             // for path
#include <map>                    // for map
#include <utility>                // for move, swap
#include <vector>                 // for vector
#include <vulkan/vulkan_core.h>   // for VK_FALSE, VkStructureType, VkColor...

namespace engine::core {

class RenderingPipelineMaker {
private:
  std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
  std::map<Shader::Stage, Shader> m_shader_modules;

  VkPipelineInputAssemblyStateCreateInfo m_input_assembly;
  VkPipelineRasterizationStateCreateInfo m_rasterizer;
  VkPipelineColorBlendAttachmentState m_color_blend_attachment;
  VkPipelineMultisampleStateCreateInfo m_multisampling;
  VkPipelineLayout m_pipeline_layout;
  VkPipelineDepthStencilStateCreateInfo m_depth_stencil;
  VkPipelineRenderingCreateInfo m_render_info;
  VkFormat m_color_attachment_format;
  VkDevice m_device;

public:
  RenderingPipelineMaker(VkDevice device) {
    reset();
    m_device = device;
  };

  RenderingPipelineMaker &reset();

  RenderingPipelineMaker &
  set_shaders(const std::map<Shader::Stage, std::filesystem::path> &shaders);

  RenderingPipelineMaker &set_input_topology(VkPrimitiveTopology topology);

  RenderingPipelineMaker &set_polygon_mode(VkPolygonMode mode);

  RenderingPipelineMaker &set_cull_mode(VkCullModeFlags cull_mode,
                                        VkFrontFace front_face);

  RenderingPipelineMaker &set_no_multisampling();

  RenderingPipelineMaker &disable_blending();

  RenderingPipelineMaker &set_color_attachment_format(VkFormat format);

  RenderingPipelineMaker &set_depth_format(VkFormat format);

  RenderingPipelineMaker &disable_depthtest();

  RenderingPipelineMaker &set_pipeline_layout(VkPipelineLayout layout);

  VkDestroyable<VkPipelineWrapper>
  make_rendering_pipeline(VkRenderPass render_pass) const;

  RenderingPipelineMaker(const RenderingPipelineMaker &) = delete;
  RenderingPipelineMaker &operator=(const RenderingPipelineMaker &) = delete;

  RenderingPipelineMaker(RenderingPipelineMaker &&other) {
    *this = std::move(other);
  }

  RenderingPipelineMaker &operator=(RenderingPipelineMaker &&other) {
    if (this != &other) {
      reset();
      std::swap(*this, other);
    }
    return *this;
  }

  ~RenderingPipelineMaker() = default;
};

VkDestroyable<VkPipelineLayoutWrapper>
make_default_pipeline_layout(VkDevice device);

} // namespace engine::core
