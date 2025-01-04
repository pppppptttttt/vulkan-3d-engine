#include "renderer.hpp"
#include "engine_exceptions.hpp"
#include "meta.hpp"
#include "physical_device_queries.hpp"
#include "window.hpp"
#include <SDL3/SDL_vulkan.h>
#include <cassert>
#include <cstring>
#include <print>
#include <set>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace engine::core {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
  std::println(stderr, "Validation layer: {}", pCallbackData->pMessage);
  return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info() noexcept {
  return VkDebugUtilsMessengerCreateInfoEXT{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debug_callback,
      .pUserData = nullptr};
}

bool validation_layers_supported() {
  unsigned layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

  return std::all_of(
      VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end(),
      [&layers](const auto &layer_name) {
        return std::find_if(layers.begin(), layers.end(),
                            [&layer_name](const auto &p) {
                              return std::strcmp(p.layerName, layer_name) == 0;
                            }) != layers.end();
      });
}

VkInstance make_instance() {
  VkInstance instance = VK_NULL_HANDLE;
  if constexpr (ENABLE_VALIDATION_LAYERS) {
    if (!validation_layers_supported()) {
      throw exceptions::ValidationLayersNotAvailiableError();
    }
  }

  const VkApplicationInfo app_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = APPLICATION_NAME.data(),
      .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(0, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = nullptr,
  };

  unsigned window_ext_count = 0;
  auto window_ext_names_ptr =
      SDL_Vulkan_GetInstanceExtensions(&window_ext_count);
  if (!window_ext_names_ptr) {
    throw exceptions::AcquireWindowExtensionsError{};
  }
  auto window_ext_names = std::span(window_ext_names_ptr, window_ext_count);

  std::vector<const char *> required_ext_names(window_ext_names.begin(),
                                               window_ext_names.end());
  if constexpr (ENABLE_VALIDATION_LAYERS) {
    required_ext_names.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  create_info.enabledExtensionCount = required_ext_names.size();
  create_info.ppEnabledExtensionNames = required_ext_names.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};

  if constexpr (ENABLE_VALIDATION_LAYERS) {
    create_info.enabledLayerCount = VALIDATION_LAYERS.size();
    create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    debug_create_info = get_debug_messenger_create_info();
    create_info.pNext = &debug_create_info;
  } else {
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;
  }

  if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    throw exceptions::InstanceCreationError{};
  }
  return instance;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  // NOLINTNEXTLINE
  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  // NOLINTNEXTLINE
  auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

VkDebugUtilsMessengerEXT make_debug_messenger(VkInstance instance) {
  if constexpr (!ENABLE_VALIDATION_LAYERS) {
    return VK_NULL_HANDLE;
  }

  VkDebugUtilsMessengerEXT debug_messenger{};
  const VkDebugUtilsMessengerCreateInfoEXT create_info =
      get_debug_messenger_create_info();
  if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr,
                                   &debug_messenger) != VK_SUCCESS) {
    throw exceptions::DebugMessengerCreationError{};
  }
  return debug_messenger;
}

VkSurfaceKHR make_surface(VkInstance instance, SDL_Window *window) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
    throw exceptions::SurfaceCreationError(SDL_GetError());
  }
  return surface;
}

VkDevice make_logical_device(VkPhysicalDevice physical_device,
                             VkSurfaceKHR surface) {
  const auto ind = find_queue_families(physical_device, surface);
  assert(ind.present_family && ind.graphics_family && ind.transfer_family);
  const float queue_priority = 1.0f;

  const std::set<unsigned> queue_families{
      *ind.present_family, *ind.graphics_family, *ind.transfer_family};
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  queue_create_infos.reserve(queue_families.size());
  for (const unsigned family : queue_families) {
    const VkDeviceQueueCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority};
    queue_create_infos.emplace_back(info);
  }

  VkPhysicalDeviceFeatures features{};
  features.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = static_cast<unsigned>(queue_create_infos.size()),
      .pQueueCreateInfos = queue_create_infos.data(),
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = DEVICE_EXTENSIONS.size(),
      .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
      .pEnabledFeatures = &features,
  };

  if constexpr (ENABLE_VALIDATION_LAYERS) {
    create_info.enabledLayerCount = VALIDATION_LAYERS.size();
    create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  }

  VkDevice device = VK_NULL_HANDLE;
  if (vkCreateDevice(physical_device, &create_info, nullptr, &device) !=
      VK_SUCCESS) {
    throw exceptions::LogicalDeviceCreationError{};
  }
  return device;
}

} // namespace

Renderer::Renderer(const Window &window)
    : m_instance(make_instance()),
      m_debug_messenger(make_debug_messenger(m_instance), m_instance,
                        DestroyDebugUtilsMessengerEXT),
      m_surface(make_surface(m_instance, window.handle), m_instance,
                vkDestroySurfaceKHR),
      m_physical_device(choose_physical_device(m_instance, m_surface)),
      m_device(make_logical_device(m_physical_device, m_surface)),
      m_graphics_queue(m_physical_device, m_surface, m_device,
                       CommandQueue::Kind::GRAPHICS),
      m_present_queue(m_physical_device, m_surface, m_device,
                      CommandQueue::Kind::PRESENT),
      m_transfer_queue(m_physical_device, m_surface, m_device,
                       CommandQueue::Kind::TRANSFER),
      m_swapchain(m_device, m_physical_device, m_surface, window),
      m_graphics_pipeline(
          m_device, m_swapchain,
          {{Shader::Stage::VERTEX, "triangle.vert.glsl.spv"},
           {Shader::Stage::FRAGMENT, "triangle.frag.glsl.spv"}}),
      /*m_current_frame(0),*/
      m_command_pool(m_device, m_physical_device, m_surface),
      m_command_buffers(m_command_pool.make_command_buffers(1).front()),
      m_render_fence(m_device), m_swapchain_semaphore(m_device),
      m_render_semaphore(m_device) {}

void Renderer::render_frame() {
  m_render_fence.wait();
  m_render_fence.reset();

  unsigned image_index = 0;
  vkAcquireNextImageKHR(m_device, m_swapchain.swapchain(),
                        std::numeric_limits<std::uint64_t>::max(),
                        m_swapchain_semaphore.semaphore(), VK_NULL_HANDLE,
                        &image_index);

  m_command_buffers.reset();
  auto draw_commands = [this, image_index](VkCommandBuffer command_buffer) {
    const std::array<VkClearValue, 2> clear_values{
        {{{{0.05f, 0.05f, 0.05f, 1.0f}}}, {{{1.0f, 0}}}}};

    const VkRenderPassBeginInfo render_pass_begin{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = m_graphics_pipeline.render_pass(),
        .framebuffer = m_swapchain.framebuffers()[image_index],
        .renderArea = {{0, 0}, m_swapchain.extent()},
        .clearValueCount = static_cast<unsigned>(clear_values.size()),
        .pClearValues = clear_values.data()};

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_graphics_pipeline.pipeline());
    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);
  };
  m_command_buffers.record(draw_commands);

  VkSemaphore wait_semaphores[] = {m_swapchain_semaphore.semaphore()};
  VkCommandBuffer command_buffers[] = {m_command_buffers.buffer()};
  VkSemaphore signal_semaphores[] = {m_render_semaphore.semaphore()};
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  const VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                 .pNext = nullptr,
                                 .waitSemaphoreCount = 1,
                                 .pWaitSemaphores = wait_semaphores,
                                 .pWaitDstStageMask = wait_stages,
                                 .commandBufferCount = 1,
                                 .pCommandBuffers = command_buffers,
                                 .signalSemaphoreCount = 1,
                                 .pSignalSemaphores = signal_semaphores};
  m_graphics_queue.submit(submit_info, m_render_fence.fence());
  /*++m_current_frame;*/
  /*m_current_frame %= FRAME_OVERLAP;*/

  VkSwapchainKHR swapchain_ptr[] = {m_swapchain.swapchain()};
  const VkPresentInfoKHR present_info{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signal_semaphores,
      .swapchainCount = 1,
      .pSwapchains = swapchain_ptr,
      .pImageIndices = &image_index,
      .pResults = nullptr,
  };

  vkQueuePresentKHR(m_present_queue.queue(), &present_info);
  /*std::exit(0);*/
}

} // namespace engine::core
