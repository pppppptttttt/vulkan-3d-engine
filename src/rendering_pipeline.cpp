#include "rendering_pipeline.hpp"
#include "engine_exceptions.hpp" // for PipelineLayoutCreationError, Render...
#include <cstdio>                // for stderr
#include <print>                 // for println
#include <stdexcept>             // for out_of_range
#include <vector>                // for vector
#include <vulkan/vulkan_core.h>  // for VkStructureType, VkShaderStageFlagBits

namespace engine::core {

VkDestroyable<VkPipelineLayoutWrapper>
make_default_pipeline_layout(VkDevice device) {
  const VkPipelineLayoutCreateInfo layout_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,   // Optional
      .pPushConstantRanges = nullptr // Optional
  };
  VkPipelineLayout layout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &layout_info, nullptr, &layout) !=
      VK_SUCCESS) {
    throw exceptions::PipelineLayoutCreationError{};
  }
  return {layout, device};
}

RenderingPipelineMaker &RenderingPipelineMaker::set_shaders(
    const std::map<Shader::Stage, std::filesystem::path> &shaders) {
  for (const auto &[stage, filename] : shaders) {
    m_shader_modules.emplace(stage, Shader(m_device, filename));
  }

  try {
    const VkPipelineShaderStageCreateInfo vertex_shader_stage_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = m_shader_modules.at(Shader::Stage::VERTEX).get_module(),
        .pName = "main",
        .pSpecializationInfo = nullptr};

    const VkPipelineShaderStageCreateInfo fragment_shader_stage_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = m_shader_modules.at(Shader::Stage::FRAGMENT).get_module(),
        .pName = "main",
        .pSpecializationInfo = nullptr};

    m_shader_stages = {vertex_shader_stage_info, fragment_shader_stage_info};

    if (auto it = m_shader_modules.find(Shader::Stage::GEOMETRY);
        it != m_shader_modules.end()) {
      m_shader_stages.push_back(
          {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .pNext = nullptr,
           .flags = 0,
           .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
           .module = it->second.get_module(),
           .pName = "main",
           .pSpecializationInfo = nullptr});
    }
    if (auto it = m_shader_modules.find(Shader::Stage::TESSELATION_CONTROL);
        it != m_shader_modules.end()) {
      m_shader_stages.push_back(
          {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .pNext = nullptr,
           .flags = 0,
           .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
           .module = it->second.get_module(),
           .pName = "main",
           .pSpecializationInfo = nullptr});
    }
    if (auto it = m_shader_modules.find(Shader::Stage::TESSELATION_EVAL);
        it != m_shader_modules.end()) {
      m_shader_stages.push_back(
          {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .pNext = nullptr,
           .flags = 0,
           .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
           .module = it->second.get_module(),
           .pName = "main",
           .pSpecializationInfo = nullptr});
    }
  } catch (const std::out_of_range &) {
    std::println(stderr, "WARNING: weird shader stagets in pipeline maker!");
  }
  return *this;
}

RenderingPipelineMaker &RenderingPipelineMaker::reset() {
  m_shader_stages.clear();
  m_shader_modules.clear();

  m_input_assembly = {};
  m_rasterizer = {};
  m_color_blend_attachment = {};
  m_multisampling = {};
  m_pipeline_layout = {};
  m_depth_stencil = {};
  m_render_info = {};
  m_color_attachment_format = {};
  m_device = {};

  m_input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  m_rasterizer.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_depth_stencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  return *this;
}
RenderingPipelineMaker &
RenderingPipelineMaker::set_input_topology(VkPrimitiveTopology topology) {
  m_input_assembly.topology = topology;
  m_input_assembly.primitiveRestartEnable = VK_FALSE;
  return *this;
}

RenderingPipelineMaker &
RenderingPipelineMaker::set_polygon_mode(VkPolygonMode mode) {
  m_rasterizer.polygonMode = mode;
  m_rasterizer.lineWidth = 1.0f;
  return *this;
}
RenderingPipelineMaker &
RenderingPipelineMaker::set_cull_mode(VkCullModeFlags cull_mode,
                                      VkFrontFace front_face) {
  m_rasterizer.cullMode = cull_mode;
  m_rasterizer.frontFace = front_face;
  return *this;
}

RenderingPipelineMaker &RenderingPipelineMaker::set_no_multisampling() {
  m_multisampling.sampleShadingEnable = VK_FALSE;
  m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  m_multisampling.minSampleShading = 1.0f;
  m_multisampling.pSampleMask = nullptr;
  m_multisampling.alphaToCoverageEnable = VK_FALSE;
  m_multisampling.alphaToOneEnable = VK_FALSE;
  return *this;
}

RenderingPipelineMaker &RenderingPipelineMaker::disable_blending() {
  m_color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  m_color_blend_attachment.blendEnable = VK_FALSE;
  return *this;
}

RenderingPipelineMaker &
RenderingPipelineMaker::set_color_attachment_format(VkFormat format) {
  m_color_attachment_format = format;
  m_render_info.colorAttachmentCount = 1;
  m_render_info.pColorAttachmentFormats = &m_color_attachment_format;
  return *this;
}

RenderingPipelineMaker &
RenderingPipelineMaker::set_depth_format(VkFormat format) {
  m_render_info.depthAttachmentFormat = format;
  return *this;
}

RenderingPipelineMaker &RenderingPipelineMaker::disable_depthtest() {
  m_depth_stencil.depthTestEnable = VK_FALSE;
  m_depth_stencil.depthWriteEnable = VK_FALSE;
  m_depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
  m_depth_stencil.depthBoundsTestEnable = VK_FALSE;
  m_depth_stencil.stencilTestEnable = VK_FALSE;
  m_depth_stencil.front = {};
  m_depth_stencil.back = {};
  m_depth_stencil.minDepthBounds = 0.f;
  m_depth_stencil.maxDepthBounds = 1.f;
  return *this;
}

RenderingPipelineMaker &
RenderingPipelineMaker::set_pipeline_layout(VkPipelineLayout layout) {
  m_pipeline_layout = layout;
  return *this;
}

VkDestroyable<VkPipelineWrapper>
RenderingPipelineMaker::make_rendering_pipeline(
    VkRenderPass render_pass) const {
  VkPipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.pNext = nullptr;

  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineColorBlendStateCreateInfo color_blending = {};
  color_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.pNext = nullptr;

  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &m_color_blend_attachment;

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = &m_render_info;

  pipeline_info.stageCount = static_cast<unsigned>(m_shader_stages.size());
  pipeline_info.pStages = m_shader_stages.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &m_input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &m_rasterizer;
  pipeline_info.pMultisampleState = &m_multisampling;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDepthStencilState = &m_depth_stencil;
  pipeline_info.layout = m_pipeline_layout;
  pipeline_info.renderPass = render_pass;

  VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT,
                            VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_info = {};
  dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_info.pDynamicStates = &state[0];
  dynamic_info.dynamicStateCount = 2;

  pipeline_info.pDynamicState = &dynamic_info;

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &pipeline) != VK_SUCCESS) {
    throw exceptions::RenderingPipelineCreationError{};
  }
  return {pipeline, m_device};
}

} // namespace engine::core
