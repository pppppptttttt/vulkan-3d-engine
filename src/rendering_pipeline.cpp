#include "rendering_pipeline.hpp"
#include "engine_exceptions.hpp" // for PipelineLayoutCreationError, Render...
#include <array>                 // for array
#include <cstdio>                // for stderr
#include <print>                 // for println
#include <span>                  // for span
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
  m_vertex_input_info = {};

  m_input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  m_rasterizer.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_depth_stencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  m_vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
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
  const VkPipelineViewportStateCreateInfo viewport_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr,
  };

  const VkPipelineColorBlendStateCreateInfo color_blending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &m_color_blend_attachment,
      .blendConstants = {},
  };

  /*const VkPipelineVertexInputStateCreateInfo vertex_input_info = {*/
  /*    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,*/
  /*    .pNext = nullptr,*/
  /*    .flags = 0,*/
  /*    .vertexBindingDescriptionCount = 0,*/
  /*    .pVertexBindingDescriptions = nullptr,*/
  /*    .vertexAttributeDescriptionCount = 0,*/
  /*    .pVertexAttributeDescriptions = nullptr,*/
  /*};*/

  std::array<VkDynamicState, 2> state = {VK_DYNAMIC_STATE_VIEWPORT,
                                         VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_info = {};
  dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_info.pDynamicStates = state.data();
  dynamic_info.dynamicStateCount = static_cast<unsigned>(state.size());

  const VkGraphicsPipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &m_render_info,
      .flags = 0,
      .stageCount = static_cast<unsigned>(m_shader_stages.size()),
      .pStages = m_shader_stages.data(),
      .pVertexInputState = &m_vertex_input_info,
      .pInputAssemblyState = &m_input_assembly,
      .pTessellationState = nullptr,
      .pViewportState = &viewport_state,
      .pRasterizationState = &m_rasterizer,
      .pMultisampleState = &m_multisampling,
      .pDepthStencilState = &m_depth_stencil,
      .pColorBlendState = &color_blending,
      .pDynamicState = &dynamic_info,
      .layout = m_pipeline_layout,
      .renderPass = render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = 0,
  };

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &pipeline) != VK_SUCCESS) {
    throw exceptions::RenderingPipelineCreationError{};
  }
  return {pipeline, m_device};
} // namespace engine::core

} // namespace engine::core
