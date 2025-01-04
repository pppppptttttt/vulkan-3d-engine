#pragma once

#include "engine_exceptions.hpp"
#include "meta.hpp"

#include <SDL3/SDL.h>

namespace engine::core {

struct Window {
  // NOLINTBEGIN
  SDL_Window *handle = nullptr;
  int width = 800;
  int height = 600;
  // NOLINTEND

  Window() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      throw exceptions::WindowCreationError(SDL_GetError());
    }

    handle = SDL_CreateWindow(APPLICATION_NAME.data(), width, height,
                              SDL_WINDOW_VULKAN /*| SDL_WINDOW_RESIZABLE*/);

    if (!handle) {
      throw exceptions::WindowCreationError(SDL_GetError());
    }
  }

  ~Window() {
    SDL_DestroyWindow(handle);
    SDL_Quit();
  }

  Window(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&) = delete;
};

} // namespace engine::core
