#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "render_object.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include <chrono>
#include <print>
#include <vulkan/vulkan_core.h>

namespace samples {

class Triangle final : public engine::RenderObject {
private:
  /*std::vector<engine::resources::Vertex> m_vertices = {*/
  /*    {{-0.5f, -0.5f, 0.0f}, {}, {}, {1.0f, 0.0f, 1.0f, 1.0f}},*/
  /*    {{0.5f, -0.5f, 0.0f}, {}, {}, {0.0f, 1.0f, 1.0f, 1.0f}},*/
  /*    {{0.5f, 0.5f, 0.0f}, {}, {}, {0.0f, 0.0f, 1.0f, 1.0f}},*/
  /*    {{-0.5f, 0.5f, 0.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f}}};*/
  /*std::vector<unsigned> m_indices = {0, 1, 2, 2, 3, 0};*/
  std::vector<engine::resources::Vertex> m_vertices = {
      {{-0.5f, 0.5f, 0.0f}, {}, {}, {1.0f, 0.0f, 1.0f}},
      {{0.0f, -0.5f, 0.0f}, {}, {}, {0.0f, 1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.0f}, {}, {}, {0.0f, 0.0f, 1.0f}},
  };
  std::vector<unsigned> m_indices = {0, 1, 2};

  engine::resources::Material m_material;
  engine::resources::Mesh m_mesh;

  struct PushConstant {
    glm::mat4 MVP{1.0f};
    float time = 0.0f;
  } m_push_constants;
  glm::vec3 translated{0.0f};

  using enum engine::core::Shader::Stage;

  const std::chrono::time_point<std::chrono::high_resolution_clock>
      m_ctor_time_point = std::chrono::high_resolution_clock::now();

public:
  Triangle(engine::core::Renderer &renderer)
      : m_material(renderer,
                   {{VERTEX, "triangle.vert.glsl.spv"},
                    {FRAGMENT, "triangle.frag.glsl.spv"}},
                   &m_push_constants, sizeof(m_push_constants),
                   VK_SHADER_STAGE_VERTEX_BIT),
        m_mesh(renderer, m_vertices, m_indices, &m_material) {
    renderer.submit_mesh(&m_mesh);
  }

  void on_render_frame() final {
    const auto now = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> elapsed = now - m_ctor_time_point;
    translated.x = sin(elapsed.count()) * 2.0f;
    m_push_constants.MVP =
        glm::scale(glm::mat4{1.0f}, glm::vec3(0.3f, 0.3f, 0.3f)) *
        glm::rotate(glm::mat4{1.0f}, elapsed.count() / 2.0f,
                    glm::vec3(0.0f, 0.0f, 1.0f));
    /*glm::translate(glm::mat4{1.0f}, translated);*/
    m_push_constants.time = elapsed.count();
  }
};

} // namespace samples
