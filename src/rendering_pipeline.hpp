#pragma once

#include "shader.hpp"             // for Shader
#include "vulkan_destroyable.hpp" // for VkDestroyable, VkPipelineLayoutWra...
#include <cstddef>                // for size_t
#include <filesystem>             // for path
#include <map>                    // for map, swap
#include <span>                   // for span
#include <utility>                // for swap, move
#include <vector>                 // for allocator, vector, swap
#include <vulkan/vulkan_core.h>   // for VkDevice, VkDevice_T, VkPipelineLa...

namespace engine::core {

class RenderingPipelineMaker {
private:
  std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
  std::map<Shader::Stage, Shader> m_shader_modules;

  VkPipelineInputAssemblyStateCreateInfo m_input_assembly{};
  VkPipelineRasterizationStateCreateInfo m_rasterizer{};
  VkPipelineColorBlendAttachmentState m_color_blend_attachment{};
  VkPipelineMultisampleStateCreateInfo m_multisampling{};
  VkPipelineLayout m_pipeline_layout{};
  VkPipelineDepthStencilStateCreateInfo m_depth_stencil{};
  VkPipelineRenderingCreateInfo m_render_info{};
  VkFormat m_color_attachment_format{};
  VkVertexInputBindingDescription m_vertex_binding{};
  std::vector<VkVertexInputAttributeDescription> m_vertex_attributes;
  VkPipelineVertexInputStateCreateInfo m_vertex_input_info{};
  VkDevice m_device = VK_NULL_HANDLE;

public:
  RenderingPipelineMaker(VkDevice device) : m_device(device) { reset(); };

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

  RenderingPipelineMaker &set_vertex_description(
      VkVertexInputBindingDescription binding,
      std::span<VkVertexInputAttributeDescription> attributes) {
    m_vertex_binding = binding;
    m_vertex_attributes = std::vector(attributes.begin(), attributes.end());

    m_vertex_input_info.vertexBindingDescriptionCount = 1;
    m_vertex_input_info.pVertexBindingDescriptions = &m_vertex_binding;

    m_vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<unsigned>(m_vertex_attributes.size());
    m_vertex_input_info.pVertexAttributeDescriptions =
        m_vertex_attributes.data();
    return *this;
  }

  VkDestroyable<VkPipelineWrapper>
  make_rendering_pipeline(VkRenderPass render_pass) const;

  RenderingPipelineMaker(const RenderingPipelineMaker &) = delete;
  RenderingPipelineMaker &operator=(const RenderingPipelineMaker &) = delete;

  RenderingPipelineMaker(RenderingPipelineMaker &&other) noexcept {
    *this = std::move(other);
  }

  RenderingPipelineMaker &operator=(RenderingPipelineMaker &&other) noexcept {
    if (this != &other) {
      reset();
      std::swap(m_shader_stages, other.m_shader_stages);
      std::swap(m_shader_modules, other.m_shader_modules);
      std::swap(m_input_assembly, other.m_input_assembly);
      std::swap(m_rasterizer, other.m_rasterizer);
      std::swap(m_color_blend_attachment, other.m_color_blend_attachment);
      std::swap(m_multisampling, other.m_multisampling);
      std::swap(m_pipeline_layout, other.m_pipeline_layout);
      std::swap(m_depth_stencil, other.m_depth_stencil);
      std::swap(m_render_info, other.m_render_info);
      std::swap(m_color_attachment_format, other.m_color_attachment_format);
      std::swap(m_vertex_binding, other.m_vertex_binding);
      std::swap(m_vertex_attributes, other.m_vertex_attributes);
      std::swap(m_vertex_input_info, other.m_vertex_input_info);
      std::swap(m_device, other.m_device);
    }
    return *this;
  }

  ~RenderingPipelineMaker() = default;
};

VkPipelineLayoutCreateInfo make_default_pipeline_layout_create_info();

VkDestroyable<VkPipelineLayoutWrapper>
make_default_pipeline_layout(VkDevice device);

template <typename T>
  requires(sizeof(T) <= 128)
struct PushConstant {
  T data;
  VkShaderStageFlags stages;
};

class PipelineLayoutMaker {
private:
  VkPipelineLayoutCreateInfo m_layout_info =
      make_default_pipeline_layout_create_info();
  VkDevice m_device = VK_NULL_HANDLE;
  VkPushConstantRange m_range{};

public:
  PipelineLayoutMaker(VkDevice device) : m_device(device) {}

  PipelineLayoutMaker &add_push_constant(VkShaderStageFlags stage_flags,
                                         std::size_t size) {
    m_range = {
        .stageFlags = stage_flags,
        .offset = 0,
        .size = static_cast<unsigned>(size),
    };
    m_layout_info.pPushConstantRanges = &m_range;
    m_layout_info.pushConstantRangeCount = 1;
    return *this;
  }

  [[nodiscard]] VkDestroyable<VkPipelineLayoutWrapper>
  make_pipeline_layout() const;
};

} // namespace engine::core
