#include "application.hpp"

namespace engine {

Application::Application() : m_renderer(m_window) {}

void Application::run() {
  SDL_Event ev;
  bool quit = false;
  while (!quit) {
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_EVENT_QUIT) {
        quit = true;
        break;
      } else if (ev.type == SDL_EVENT_KEY_DOWN) {
        quit = ev.key.key == SDLK_ESCAPE;
      }
    }

    if (quit) {
      break;
    }

    m_renderer.render_frame();
  }
  m_renderer.wait_idle();
}

/*Application::~Application() {}*/

} // namespace engine
