#pragma once

#include "SDL3/SDL_video.h" // for SDL_Window

namespace engine::core {

struct Window {
  // NOLINTBEGIN
  SDL_Window *handle = nullptr;
  int width = 800;
  int height = 600;
  // NOLINTEND

  Window();
  ~Window();

  Window(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&) = delete;
};

} // namespace engine::core
