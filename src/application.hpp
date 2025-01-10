#pragma once

#include "render_object.hpp" // for RenderObject
#include "renderer.hpp"      // for Renderer
#include "window.hpp"        // for Window
#include <concepts>          // for derived_from
#include <functional>        // for ref
#include <memory>            // for unique_ptr, make_unique
#include <utility>           // for forward
#include <vector>            // for vector

namespace engine {

class Application {
private:
  core::Window m_window;
  core::Renderer m_renderer;
  std::vector<std::unique_ptr<RenderObject>> m_render_objects;

public:
  Application();
  void run();
  ~Application() = default;

  template <typename T, typename... Args>
    requires std::derived_from<T, RenderObject>
  auto add_render_object(Args &&...args)
      -> decltype(std::make_unique<T>(m_renderer, std::forward<Args>(args)...),
                  std::ref(*this)) {
    m_render_objects.emplace_back(
        std::make_unique<T>(m_renderer, std::forward<Args>(args)...));
    return *this;
  }

  Application(const Application &) = delete;
  Application(Application &&) noexcept = delete;
  Application &operator=(const Application &) = delete;
  Application &operator=(Application &&) noexcept = delete;
};

} // namespace engine
