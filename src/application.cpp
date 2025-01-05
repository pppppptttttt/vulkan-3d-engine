#include "application.hpp"
#include "SDL3/SDL_events.h"  // for SDL_EventType, SDL_PollEvent, SDL_Event
#include "SDL3/SDL_keycode.h" // for SDLK_ESCAPE

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
      } else if (ev.type == SDL_EVENT_WINDOW_RESIZED) {
        m_window.width = ev.window.data1;
        m_window.height = ev.window.data2;
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
