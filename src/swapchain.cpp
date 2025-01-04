#include "swapchain.hpp"
#include "physical_device_queries.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace engine::core {

namespace {

VkImageView make_image_view(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspect_flags) {
  const VkImageViewCreateInfo view_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .components = {},
      .subresourceRange = {.aspectMask = aspect_flags,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  VkImageView view{};
  if (vkCreateImageView(device, &view_info, nullptr, &view) != VK_SUCCESS) {
    throw exceptions::ImageViewCreationError{};
  }
  return view;
}

} // namespace

Swapchain::SupportDetails
Swapchain::get_swapchain_support_details(VkPhysicalDevice device,
                                         VkSurfaceKHR surface) {
  SupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  unsigned format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         details.formats.data());
  }

  unsigned present_modes_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &present_modes_count, nullptr);
  if (present_modes_count != 0) {
    details.present_modes.resize(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &present_modes_count, nullptr);
  }
  return details;
}

Swapchain::Swapchain(VkDevice device, VkPhysicalDevice physical_device,
                     VkSurfaceKHR surface, const Window &window) {
  auto [surface_capabilities, surface_formats, present_modes] =
      get_swapchain_support_details(physical_device, surface);

  auto surface_format_it =
      std::find_if(surface_formats.begin(), surface_formats.end(),
                   [](const VkSurfaceFormatKHR &fmt) {
                     return fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
                            fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                   });
  const VkSurfaceFormatKHR surface_format =
      surface_format_it == surface_formats.end() ? surface_formats.front()
                                                 : *surface_format_it;

  auto present_mode_it = std::find_if(
      present_modes.begin(), present_modes.end(), [](VkPresentModeKHR mode) {
        return mode == VK_PRESENT_MODE_MAILBOX_KHR;
      });
  const VkPresentModeKHR present_mode = present_mode_it == present_modes.end()
                                            ? VK_PRESENT_MODE_FIFO_KHR
                                            : *present_mode_it;

  if (surface_capabilities.currentExtent.width !=
      std::numeric_limits<unsigned>::max()) {
    m_extent = surface_capabilities.currentExtent;
  } else {
    m_extent = {std::clamp(static_cast<unsigned>(window.width),
                           surface_capabilities.minImageExtent.width,
                           surface_capabilities.maxImageExtent.width),
                std::clamp(static_cast<unsigned>(window.height),
                           surface_capabilities.minImageExtent.height,
                           surface_capabilities.maxImageExtent.height)};
  }

  unsigned image_count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      image_count > surface_capabilities.maxImageCount) {
    image_count = surface_capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = m_extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = surface_capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE};

  const QueueFamilyIndices ind = find_queue_families(physical_device, surface);
  assert(ind.graphics_family && ind.present_family);
  if (ind.graphics_family != ind.present_family) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
  }

  if (vkCreateSwapchainKHR(device, &create_info, nullptr, &m_swapchain) !=
      VK_SUCCESS) {
    throw exceptions::SwapchainCreationError{};
  }
  m_swapchain.wrapped.parent = device;
  m_swapchain.wrapped.destroy_function = vkDestroySwapchainKHR;

  m_image_format = surface_format.format;

  unsigned swapchain_image_count = 0;
  vkGetSwapchainImagesKHR(device, m_swapchain, &swapchain_image_count, nullptr);
  m_images.resize(swapchain_image_count);
  vkGetSwapchainImagesKHR(device, m_swapchain, &swapchain_image_count,
                          m_images.data());

  for (VkImage image : m_images) {
    m_image_views.emplace_back(make_image_view(device, image, m_image_format,
                                               VK_IMAGE_ASPECT_COLOR_BIT),
                               device, vkDestroyImageView);
  }
}

void Swapchain::make_framebuffers(VkDevice device, VkRenderPass render_pass) {
  for (VkImageView image_view : m_image_views) {
    const std::array<VkImageView, 1> attachments = {
        image_view}; //,
                     // depth_image_view};

    const VkFramebufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderPass = render_pass,
        .attachmentCount = static_cast<unsigned>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = m_extent.width,
        .height = m_extent.height,
        .layers = 1};

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    if (vkCreateFramebuffer(device, &create_info, nullptr, &framebuffer) !=
        VK_SUCCESS) {
      throw exceptions::FramebufferCreationError{};
    }

    m_framebuffers.emplace_back(framebuffer, device, vkDestroyFramebuffer);
  }
}

} // namespace engine::core
