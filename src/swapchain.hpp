#pragma once

#include "vulkan_destroyable.hpp"
#include "window.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace engine::core {

class Swapchain {
private:
  VkFormat m_image_format{};
  VkExtent2D m_extent{};
  std::vector<VkDestroyable<VkFramebufferWrapper>> m_framebuffers;
  VkDevice m_device;
  std::vector<VkImage> m_images;
  std::vector<VkDestroyable<VkImageViewWrapper>> m_image_views;
  VkDestroyable<VkSwapchainKHRWrapper> m_swapchain{};

  struct SupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> present_modes{};
  };

  VkSwapchainCreateInfoKHR m_create_info{};

  SupportDetails get_swapchain_support_details(VkPhysicalDevice device,
                                               VkSurfaceKHR surface);

public:
  Swapchain(VkDevice device, VkPhysicalDevice physical_device,
            VkSurfaceKHR surface, const Window &window,
            VkRenderPass render_pass = VK_NULL_HANDLE);

  // once render pass created, it bootstraps framebuffers
  void make_framebuffers(VkRenderPass render_pass);

  [[nodiscard]] VkFormat image_format() const { return m_image_format; }
  [[nodiscard]] VkExtent2D extent() const { return m_extent; }
  [[nodiscard]] VkSwapchainKHR swapchain() const { return m_swapchain; }
  [[nodiscard]] const std::vector<VkDestroyable<VkFramebufferWrapper>> &
  framebuffers() const {
    return m_framebuffers;
  }
};

} // namespace engine::core
