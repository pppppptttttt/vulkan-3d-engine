#pragma once

#include "vulkan_destroyable.hpp"
#include "window.hpp"
#include <vector>

namespace engine::core {

class Swapchain {
private:
  VkFormat m_image_format{};
  VkExtent2D m_extent{};
  VkDestroyable<VkSwapchainKHRWrapper> m_swapchain{};
  std::vector<VkImage> m_images;
  std::vector<VkDestroyable<VkImageViewWrapper>> m_image_views;
  std::vector<VkDestroyable<VkFramebufferWrapper>> m_framebuffers;

  struct SupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> present_modes{};
  };

  SupportDetails get_swapchain_support_details(VkPhysicalDevice device,
                                               VkSurfaceKHR surface);

public:
  Swapchain(VkDevice device, VkPhysicalDevice physical_device,
            VkSurfaceKHR surface, const Window &window);

  void make_framebuffers(VkDevice device, VkRenderPass render_pass);

  [[nodiscard]] VkFormat image_format() const { return m_image_format; }
  [[nodiscard]] VkExtent2D extent() const { return m_extent; }
  [[nodiscard]] VkSwapchainKHR swapchain() const { return m_swapchain; }
  [[nodiscard]] const std::vector<VkDestroyable<VkFramebufferWrapper>> &
  framebuffers() const {
    return m_framebuffers;
  }
};

} // namespace engine::core
