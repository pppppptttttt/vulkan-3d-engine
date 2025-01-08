#pragma once

#include "command_buffers.hpp" // for CommandBuffer, CommandPool
#include "mesh.hpp"
#include "queue.hpp"              // for CommandQueue
#include "swapchain.hpp"          // for Window, Swapchain
#include "synchronization.hpp"    // for Semaphore, Fence
#include "vulkan_destroyable.hpp" // for VkDestroyable, VkDebugUtilsMesseng...
#include <array>                  // for array
#include <cstddef>                // for size_t
#include <utility>
#include <vulkan/vulkan_core.h> // for vkDeviceWaitIdle, VkDevice, VkInst...

namespace engine::core {

class Renderer {
public:
  Renderer(Window &window);

  void render_frame();

  void wait_idle() const { vkDeviceWaitIdle(m_device); }

  [[nodiscard]] VkDevice device() const { return m_device; }

  [[nodiscard]] VkPhysicalDevice physical_device() const {
    return m_physical_device;
  }

  [[nodiscard]] const CommandPool &command_pool() const {
    return m_command_pool;
  }

  [[nodiscard]] const CommandPool &transfer_command_pool() const {
    return m_transfer_command_pool;
  }

  [[nodiscard]] CommandQueue &queue(CommandQueue::Kind kind) {
    using enum CommandQueue::Kind;
    switch (kind) {
    case PRESENT:
      return m_present_queue;
    case GRAPHICS:
      return m_graphics_queue;
    case TRANSFER:
      return m_transfer_queue;
    default:
      std::unreachable();
    }
  }

  Renderer(const Renderer &) = delete;
  Renderer(Renderer &&) noexcept = delete;
  Renderer &operator=(const Renderer &) = delete;
  Renderer &operator=(Renderer &&) noexcept = delete;

  ~Renderer() = default;

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

  VkDestroyable<VkRenderPassWrapper> m_render_pass;
  VkDestroyable<VkPipelineLayoutWrapper> m_pipeline_layout;
  VkDestroyable<VkPipelineWrapper> m_pipeline;

  CommandPool m_command_pool;
  CommandPool m_transfer_command_pool;
  std::array<CommandBuffer, FRAME_OVERLAP> m_command_buffers;

  std::array<Fence, FRAME_OVERLAP> m_render_fences;
  std::array<Semaphore, FRAME_OVERLAP> m_swapchain_semaphores;
  std::array<Semaphore, FRAME_OVERLAP> m_render_semaphores;

  resources::Mesh m_mesh;
};

} // namespace engine::core
