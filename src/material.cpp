#include "material.hpp"
#include "renderer.hpp"           // for Renderer
#include "rendering_pipeline.hpp" // for RenderingPipelineMaker, PipelineLa...
#include "shader.hpp"             // for Shader
#include "swapchain.hpp"          // for Swapchain
#include "vertex.hpp"             // for Vertex
#include <array>                  // for array
#include <span>                   // for span
#include <vulkan/vulkan_core.h>   // for VkCullModeFlagBits, VkFormat, VkFr...

namespace engine::resources {

Material::Material(
    core::Renderer &renderer,
    const std::map<core::Shader::Stage, std::filesystem::path> &shaders,
    void *push_constant_data, std::size_t push_constant_size,
    VkShaderStageFlags push_constant_stages)
    : m_push_constant_data(push_constant_data),
      m_push_constant_size(push_constant_size),
      m_push_constant_stages(push_constant_stages) {
  core::PipelineLayoutMaker layout_maker(renderer.device());
  if (push_constant_data) {
    layout_maker.add_push_constant(push_constant_stages, push_constant_size);
  }
  m_pipeline_layout = layout_maker.make_pipeline_layout();

  core::RenderingPipelineMaker pipeline_maker(renderer.device());
  m_pipeline =
      pipeline_maker.set_pipeline_layout(m_pipeline_layout)
          .set_shaders(shaders)
          .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .set_polygon_mode(VK_POLYGON_MODE_FILL)
          .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
          .set_no_multisampling()
          .disable_blending()
          .disable_depthtest()
          .set_color_attachment_format(renderer.swapchain().image_format())
          .set_depth_format(VK_FORMAT_UNDEFINED)
          .set_vertex_description(
              resources::Vertex::binding_description(),
              std::span(resources::Vertex::attribute_description().data(),
                        resources::Vertex::attribute_description().size()))
          .make_rendering_pipeline(renderer.render_pass());
}

} // namespace engine::resources
