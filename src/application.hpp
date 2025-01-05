#pragma once

#include "renderer.hpp" // for Renderer
#include "window.hpp"   // for Window

namespace engine {

class Application {
private:
  core::Window m_window;
  core::Renderer m_renderer;

public:
  Application();
  void run();
  ~Application() = default;

  Application(const Application &) = delete;
  Application(Application &&) noexcept = delete;
  Application &operator=(const Application &) = delete;
  Application &operator=(Application &&) noexcept = delete;
};

} // namespace engine
