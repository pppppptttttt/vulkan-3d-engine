#include "window.hpp"
#include "SDL3/SDL_error.h"      // for SDL_GetError
#include "SDL3/SDL_init.h"       // for SDL_Init, SDL_Quit, SDL_INIT_VIDEO
#include "engine_exceptions.hpp" // for WindowCreationError
#include "meta.hpp"              // for APPLICATION_NAME

namespace engine::core {

Window::Window() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw exceptions::WindowCreationError(SDL_GetError());
  }

  handle = SDL_CreateWindow(APPLICATION_NAME.data(), width, height,
                            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  if (!handle) {
    throw exceptions::WindowCreationError(SDL_GetError());
  }
}

Window::~Window() {
  SDL_DestroyWindow(handle);
  SDL_Quit();
}

} // namespace engine::core
