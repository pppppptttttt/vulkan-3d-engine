#include "renderer.hpp"
#include "SDL3/SDL_error.h"            // for SDL_GetError
#include "SDL3/SDL_video.h"            // for SDL_Window
#include "engine_exceptions.hpp"       // for AcquireWindowExtensionsError
#include "glm/detail/qualifier.hpp"    // for qualifier
#include "glm/vec3.hpp"                // for vec<>::vec<3, type-parameter-...
#include "glm/vec4.hpp"                // for vec<>::vec<4, type-parameter-...
#include "mesh.hpp"                    // for Mesh
#include "meta.hpp"                    // for VALIDATION_LAYERS, ENABLE_VAL...
#include "physical_device_queries.hpp" // for QueueFamilyIndices, choose_ph...
#include "rendering_pipeline.hpp"      // for RenderingPipelineMaker, make_...
#include "shader.hpp"                  // for Shader
#include "synchronization.hpp"         // for Semaphore, Fence
#include "vertex.hpp"                  // for Vertex
#include "vulkan_buffers.hpp"          // for Buffer
#include "window.hpp"                  // for Window
#include <SDL3/SDL_vulkan.h>           // for SDL_Vulkan_CreateSurface, SDL...
#include <algorithm>                   // for all_of, find_if
#include <array>                       // for array
#include <cassert>                     // for assert
#include <cstdint>                     // for uint64_t
#include <cstdio>                      // for stderr
#include <cstring>                     // for strcmp
#include <filesystem>                  // for path
#include <limits>                      // for numeric_limits
#include <map>                         // for _Rb_tree_const_iterator, map
#include <optional>                    // for optional
#include <print>                       // for println
#include <set>                         // for set
#include <span>                        // for span
#include <utility>                     // for pair
#include <vector>                      // for vector
#include <vulkan/vk_platform.h>        // for VKAPI_ATTR, VKAPI_CALL
#include <vulkan/vulkan_core.h>        // for VkStructureType, VkResult

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

VkRenderPass make_render_pass(VkDevice device, Swapchain &swapchain) {
  const VkAttachmentDescription color_attachment{
      .flags = 0,
      .format = swapchain.image_format(),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

  const VkAttachmentReference color_attachment_reference{
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  /*const VkAttachmentDescription depth_attachment{*/
  /*    .flags = 0,*/
  /*    .format = find_depth_format(),*/
  /*    .samples = VK_SAMPLE_COUNT_1_BIT,*/
  /*    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,*/
  /*    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,*/
  /*    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,*/
  /*    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,*/
  /*    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,*/
  /*    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};*/
  /**/
  /*const VkAttachmentReference depth_attachment_reference{*/
  /*    .attachment = 1,*/
  /*    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};*/

  const VkSubpassDescription subpass{
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_reference,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = nullptr, //&depth_attachment_reference,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr};

  const VkSubpassDependency subpass_dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0};

  std::array<VkAttachmentDescription, 1> attachments = {color_attachment/*,
                                                        depth_attachment*/};

  const VkRenderPassCreateInfo render_psas_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = static_cast<unsigned>(attachments.size()),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &subpass_dependency};

  VkRenderPass render_pass = VK_NULL_HANDLE;
  if (vkCreateRenderPass(device, &render_psas_info, nullptr, &render_pass) !=
      VK_SUCCESS) {
    throw exceptions::RenderPassCreationError{};
  }
  swapchain.make_framebuffers(render_pass);
  return render_pass;
}

} // namespace

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

Renderer::Renderer(Window &window)
    : m_current_frame(0), m_window(window), m_instance(make_instance()),
      m_debug_messenger(make_debug_messenger(m_instance), m_instance),
      m_surface(make_surface(m_instance, window.handle), m_instance),
      m_physical_device(choose_physical_device(m_instance, m_surface)),
      m_device(make_logical_device(m_physical_device, m_surface)),
      m_graphics_queue(m_physical_device, m_surface, m_device,
                       CommandQueue::Kind::GRAPHICS),
      m_present_queue(m_physical_device, m_surface, m_device,
                      CommandQueue::Kind::PRESENT),
      m_transfer_queue(m_physical_device, m_surface, m_device,
                       CommandQueue::Kind::TRANSFER),
      m_swapchain(m_device, m_physical_device, m_surface, window),
      m_render_pass(make_render_pass(m_device, m_swapchain), m_device),
      m_pipeline_layout(make_default_pipeline_layout(m_device)),
      m_command_pool(m_device, m_physical_device, m_surface),
      m_transfer_command_pool(m_device, m_physical_device, m_surface, true) {
  RenderingPipelineMaker pipeline_maker(m_device);
  m_pipeline =
      pipeline_maker.set_pipeline_layout(m_pipeline_layout)
          .set_shaders({{Shader::Stage::VERTEX, "triangle.vert.glsl.spv"},
                        {Shader::Stage::FRAGMENT, "triangle.frag.glsl.spv"}})
          .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .set_polygon_mode(VK_POLYGON_MODE_FILL)
          .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
          .set_no_multisampling()
          .disable_blending()
          .disable_depthtest()
          .set_color_attachment_format(m_swapchain.image_format())
          .set_depth_format(VK_FORMAT_UNDEFINED)
          .set_vertex_description(
              resources::Vertex::binding_description(),
              std::span(resources::Vertex::attribute_description().data(),
                        resources::Vertex::attribute_description().size()))
          .make_rendering_pipeline(m_render_pass);
  for (auto &semaphore : m_swapchain_semaphores) {
    semaphore = Semaphore(m_device);
  }
  for (auto &semaphore : m_render_semaphores) {
    semaphore = Semaphore(m_device);
  }
  for (auto &fence : m_render_fences) {
    fence = Fence(m_device);
  }

  const auto &vec_command_buffers =
      m_command_pool.make_command_buffers(FRAME_OVERLAP);
  for (std::size_t i = 0; i < FRAME_OVERLAP; ++i) {
    m_command_buffers[i] = vec_command_buffers[i];
  }

  std::vector<resources::Vertex> vertices = {
      {{0.0f, -0.5f, 0.0f}, {}, {}, {1.0f, 0.0f, 1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.0f}, {}, {}, {0.0f, 1.0f, 1.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.0f}, {}, {}, {0.0f, 0.0f, 1.0f, 1.0f}}};

  m_mesh = resources::Mesh(*this, vertices);
}

void Renderer::render_frame() {
  m_render_fences[m_current_frame].wait();

  unsigned image_index = 0;
  const VkResult acquire_result =
      vkAcquireNextImageKHR(m_device, m_swapchain.swapchain(),
                            std::numeric_limits<std::uint64_t>::max(),
                            m_swapchain_semaphores[m_current_frame].semaphore(),
                            VK_NULL_HANDLE, &image_index);

  switch (acquire_result) {
  case VK_ERROR_OUT_OF_DATE_KHR:
    vkDeviceWaitIdle(m_device);
    m_swapchain = Swapchain(m_device, m_physical_device, m_surface, m_window,
                            m_render_pass);
    return;
    break;

  case VK_SUCCESS:
  case VK_SUBOPTIMAL_KHR:
    break;

  default:
    throw exceptions::AcquireWindowExtensionsError{};
  }

  m_render_fences[m_current_frame].reset();

  m_command_buffers[m_current_frame].reset();
  auto draw_commands = [this, image_index](VkCommandBuffer command_buffer) {
    const std::array<VkClearValue, 2> clear_values{
        {{{{0.05f, 0.05f, 0.05f, 1.0f}}}, {{{1.0f, 0}}}}};

    const VkRenderPassBeginInfo render_pass_begin{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = m_render_pass,
        .framebuffer = m_swapchain.framebuffers()[image_index],
        .renderArea = {{0, 0}, m_swapchain.extent()},
        .clearValueCount = static_cast<unsigned>(clear_values.size()),
        .pClearValues = clear_values.data()};

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_pipeline);
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain.extent().width);
    viewport.height = static_cast<float>(m_swapchain.extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain.extent();
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkBuffer vertex_buffers[] = {m_mesh.vertices().buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);
  };
  m_command_buffers[m_current_frame].record(draw_commands);

  VkSemaphore wait_semaphores[] = {
      m_swapchain_semaphores[m_current_frame].semaphore()};
  VkCommandBuffer command_buffers[] = {
      m_command_buffers[m_current_frame].buffer()};
  VkSemaphore signal_semaphores[] = {
      m_render_semaphores[m_current_frame].semaphore()};
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
  m_graphics_queue.submit(submit_info,
                          m_render_fences[m_current_frame].fence());

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

  const VkResult present_result =
      vkQueuePresentKHR(m_present_queue.queue(), &present_info);

  switch (present_result) {
  case VK_ERROR_OUT_OF_DATE_KHR:
  case VK_SUBOPTIMAL_KHR:
    vkDeviceWaitIdle(m_device);
    m_swapchain = Swapchain(m_device, m_physical_device, m_surface, m_window,
                            m_render_pass);

  case VK_SUCCESS:
    break;

  default:
    throw exceptions::PresentSwapchainError{};
  }

  ++m_current_frame;
  m_current_frame %= FRAME_OVERLAP;
  /*std::exit(0);*/
}

} // namespace engine::core
