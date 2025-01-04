#pragma once

#include "rendering_pipeline.hpp"
#include "synchronization.hpp"
#include "vulkan_destroyable.hpp"
#include "window.hpp"
#include "queue.hpp"
#include "swapchain.hpp"
#include "command_buffers.hpp"
#include <vulkan/vulkan_core.h>

namespace engine::core {

class Renderer {
public:
  Renderer(const Window &window);
  ~Renderer() = default;

  void render_frame();
  void wait_idle() const { vkDeviceWaitIdle(m_device); }

  Renderer(const Renderer &) = delete;
  Renderer(Renderer &&) noexcept = delete;
  Renderer &operator=(const Renderer &) = delete;
  Renderer &operator=(Renderer &&) noexcept = delete;

private:
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
  /*static constexpr std::size_t FRAME_OVERLAP = 2;*/
  /*std::size_t m_current_frame;*/
  CommandPool m_command_pool;
  CommandBuffer m_command_buffers;
  Fence m_render_fence;
  Semaphore m_swapchain_semaphore;
  Semaphore m_render_semaphore;
};

} // namespace engine::core
