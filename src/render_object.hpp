#pragma once

namespace engine {

class RenderObject {
public:
  RenderObject() = default;
  virtual ~RenderObject() = default;
  virtual void on_render_frame() = 0;

  RenderObject(const RenderObject &) = delete;
  RenderObject(RenderObject &&) = delete;
  RenderObject &operator=(const RenderObject &) = delete;
  RenderObject &operator=(RenderObject &&) = delete;
};

} // namespace engine
