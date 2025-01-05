#pragma once

#include <stdexcept> // for runtime_error
#include <string>    // for string

namespace engine::exceptions {

struct EngineError : public std::runtime_error {
  EngineError(const std::string &error) : std::runtime_error(error) {}
};

struct WindowCreationError : EngineError {
  WindowCreationError(const std::string &msg)
      : EngineError(std::string("Window creation error: ") + msg) {}
};

struct AcquireWindowExtensionsError : EngineError {
  AcquireWindowExtensionsError()
      : EngineError("Failed to acquire extensions!") {}
};

struct ValidationLayersNotAvailiableError : EngineError {
  ValidationLayersNotAvailiableError()
      : EngineError("Validation layers are requested, but not availiable!") {}
};

struct InstanceCreationError : EngineError {
  InstanceCreationError() : EngineError("Failed to create vulkan instance!") {}
};

struct DebugMessengerCreationError : EngineError {
  DebugMessengerCreationError()
      : EngineError("Failed to create debug messenger!") {}
};

struct SuitableGPUNotFound : EngineError {
  SuitableGPUNotFound() : EngineError("Failed to find suitable GPU!") {}
};

struct SurfaceCreationError : EngineError {
  SurfaceCreationError(const std::string &msg = "")
      : EngineError("Failed to create surface! " + msg) {}
};

struct LogicalDeviceCreationError : EngineError {
  LogicalDeviceCreationError()
      : EngineError("Failed to create logical device!") {}
};

struct SwapchainCreationError : EngineError {
  SwapchainCreationError() : EngineError("Failed to create swapchain!") {}
};

struct ImageViewCreationError : EngineError {
  ImageViewCreationError() : EngineError("Failed to create image view!") {}
};

struct ShaderModuleCreationError : EngineError {
  ShaderModuleCreationError()
      : EngineError("Failed to create shader module!") {}
};

struct PipelineLayoutCreationError : EngineError {
  PipelineLayoutCreationError()
      : EngineError("Failed to create pipeline layout!") {}
};

struct RenderPassCreationError : EngineError {
  RenderPassCreationError() : EngineError("Failed to create render pass!") {}
};

struct FramebufferCreationError : EngineError {
  FramebufferCreationError() : EngineError("Failed to create framebuffer!") {}
};

struct RenderingPipelineCreationError : EngineError {
  RenderingPipelineCreationError()
      : EngineError("Failed to create graphics pipeline!") {}
};

struct CommandPoolCreationError : EngineError {
  CommandPoolCreationError() : EngineError("Failed to create command pool!") {}
};

struct CommandPoolAllocaionError : EngineError {
  CommandPoolAllocaionError()
      : EngineError("Failed to allocate command buffer!") {}
};

struct CommandPoolRecordError : EngineError {
  CommandPoolRecordError()
      : EngineError("Failed to begin recording command buffer!") {}
};

struct SyncPrimitivesCreationError : EngineError {
  SyncPrimitivesCreationError()
      : EngineError("Failed to create synchronization primitive(s)!") {}
};

struct SubmitCommandBufferError : EngineError {
  SubmitCommandBufferError()
      : EngineError("Failed to submit draw command buffer!") {}
};

struct PresentSwapchainError : EngineError {
  PresentSwapchainError() : EngineError("Failed to present swapchain image!") {}
};

} // namespace engine::exceptions
