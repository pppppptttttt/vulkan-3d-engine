#include "rendering_pipeline.hpp"
#include <vulkan/vulkan_core.h>

namespace engine::core {

RenderingPipeline::RenderingPipeline(
    VkDevice device, Swapchain &swapchain,
    const std::map<Shader::Stage, std::filesystem::path> &shaders) {
  const VkAttachmentDescription color_attachment{
      .flags = 0,
      .format = swapchain.image_format(),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

  const VkAttachmentReference color_attachment_reference{
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  /*const VkAttachmentDescription depth_attachment{*/
  /*    .flags = 0,*/
  /*    .format = find_depth_format(),*/
  /*    .samples = VK_SAMPLE_COUNT_1_BIT,*/
  /*    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,*/
  /*    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,*/
  /*    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,*/
  /*    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,*/
  /*    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,*/
  /*    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};*/
  /**/
  /*const VkAttachmentReference depth_attachment_reference{*/
  /*    .attachment = 1,*/
  /*    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};*/

  const VkSubpassDescription subpass{
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_reference,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = nullptr, //&depth_attachment_reference,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr};

  const VkSubpassDependency subpass_dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0};

  std::array<VkAttachmentDescription, 1> attachments = {color_attachment/*,
                                                        depth_attachment*/};

  const VkRenderPassCreateInfo render_psas_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = static_cast<unsigned>(attachments.size()),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &subpass_dependency};

  VkRenderPass render_pass = VK_NULL_HANDLE;
  if (vkCreateRenderPass(device, &render_psas_info, nullptr, &render_pass) !=
      VK_SUCCESS) {
    throw exceptions::RenderPassCreationError{};
  }
  m_render_pass = {render_pass, device};

  swapchain.make_framebuffers(m_render_pass);

  std::map<Shader::Stage, Shader> shader_modules;
  for (const auto &[stage, filename] : shaders) {
    shader_modules.emplace(stage, Shader(device, filename));
  }

  const VkPipelineShaderStageCreateInfo vertex_shader_stage_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shader_modules.at(Shader::Stage::VERTEX).get_module(),
      .pName = "main",
      .pSpecializationInfo = nullptr};

  const VkPipelineShaderStageCreateInfo fragment_shader_stage_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = shader_modules.at(Shader::Stage::FRAGMENT).get_module(),
      .pName = "main",
      .pSpecializationInfo = nullptr};

  std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
      vertex_shader_stage_info, fragment_shader_stage_info};

  if (auto it = shader_modules.find(Shader::Stage::GEOMETRY);
      it != shader_modules.end()) {
    shader_stages.push_back(
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .pNext = nullptr,
         .flags = 0,
         .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
         .module = shader_modules.at(Shader::Stage::GEOMETRY).get_module(),
         .pName = "main",
         .pSpecializationInfo = nullptr});
  }
  if (auto it = shader_modules.find(Shader::Stage::TESSELATION_CONTROL);
      it != shader_modules.end()) {
    shader_stages.push_back(
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .pNext = nullptr,
         .flags = 0,
         .stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
         .module =
             shader_modules.at(Shader::Stage::TESSELATION_CONTROL).get_module(),
         .pName = "main",
         .pSpecializationInfo = nullptr});
  }
  if (auto it = shader_modules.find(Shader::Stage::TESSELATION_EVAL);
      it != shader_modules.end()) {
    shader_stages.push_back(
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .pNext = nullptr,
         .flags = 0,
         .stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
         .module =
             shader_modules.at(Shader::Stage::TESSELATION_EVAL).get_module(),
         .pName = "main",
         .pSpecializationInfo = nullptr});
  }

  const VkPipelineVertexInputStateCreateInfo vertex_input_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount = 0,
      .pVertexBindingDescriptions = nullptr,
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions = nullptr};

  const VkPipelineInputAssemblyStateCreateInfo input_assembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      /*.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,*/
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  const VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapchain.extent().width),
      .height = static_cast<float>(swapchain.extent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};

  const VkRect2D scissor{.offset = {0, 0}, .extent = swapchain.extent()};

  const VkPipelineViewportStateCreateInfo viewport_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor};

  const VkPipelineRasterizationStateCreateInfo rasterizer{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f};

  const VkPipelineMultisampleStateCreateInfo multisampling{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,          // Optional
      .pSampleMask = nullptr,            // Optional
      .alphaToCoverageEnable = VK_FALSE, // Optional
      .alphaToOneEnable = VK_FALSE,      // Optional
  };

  const VkPipelineColorBlendAttachmentState color_blend_attachment{
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
      .colorBlendOp = VK_BLEND_OP_ADD,             // Optional
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
      .alphaBlendOp = VK_BLEND_OP_ADD,             // Optional
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  const VkPipelineColorBlendStateCreateInfo color_blending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY, // Optional
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f} // Optional
  };

  const VkPipelineLayoutCreateInfo pipeline_layout_create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,   // Optional
      .pPushConstantRanges = nullptr // Optional
  };

  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr,
                             &pipeline_layout) != VK_SUCCESS) {
    throw exceptions::PipelineLayoutCreationError{};
  }
  m_pipeline_layout = {pipeline_layout, device};

  const VkGraphicsPipelineCreateInfo pipeline_info{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = static_cast<unsigned>(shader_stages.size()),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_info,
      .pInputAssemblyState = &input_assembly,
      .pTessellationState = nullptr,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = nullptr, //&depth_stencil,
      .pColorBlendState = &color_blending,
      .pDynamicState = nullptr,
      .layout = m_pipeline_layout,
      .renderPass = m_render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1};

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &pipeline) != VK_SUCCESS) {
    throw exceptions::RenderingPipelineCreationError{};
  }
  m_pipeline = {pipeline, device};
}

} // namespace engine::core
