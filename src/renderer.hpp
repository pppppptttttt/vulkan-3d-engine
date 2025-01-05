#pragma once

#include "command_buffers.hpp"
#include "queue.hpp"
#include "rendering_pipeline.hpp"
#include "swapchain.hpp"
#include "synchronization.hpp"
#include "vulkan_destroyable.hpp"
#include "window.hpp"

namespace engine::core {

class Renderer {
public:
  Renderer(Window &window);

  void render_frame();
  void wait_idle() const { vkDeviceWaitIdle(m_device); }

  Renderer(const Renderer &) = delete;
  Renderer(Renderer &&) noexcept = delete;
  Renderer &operator=(const Renderer &) = delete;
  Renderer &operator=(Renderer &&) noexcept = delete;

  static constexpr std::size_t FRAME_OVERLAP = 2;

private:
  std::size_t m_current_frame;
  Window &m_window;
  VkDestroyable<VkInstance> m_instance;
  VkDestroyable<VkDebugUtilsMessengerEXTWrapper> m_debug_messenger;
  VkDestroyable<VkSurfaceKHRWrapper> m_surface;
  VkPhysicalDevice m_physical_device;
  VkDestroyable<VkDevice> m_device;
  CommandQueue m_graphics_queue;
  CommandQueue m_present_queue;
  CommandQueue m_transfer_queue;
  Swapchain m_swapchain;
  RenderingPipeline m_graphics_pipeline;
  CommandPool m_command_pool;
  std::array<CommandBuffer, FRAME_OVERLAP> m_command_buffers;
  std::array<Fence, FRAME_OVERLAP> m_render_fences;
  std::array<Semaphore, FRAME_OVERLAP> m_swapchain_semaphores;
  std::array<Semaphore, FRAME_OVERLAP> m_render_semaphores;
};

} // namespace engine::core
