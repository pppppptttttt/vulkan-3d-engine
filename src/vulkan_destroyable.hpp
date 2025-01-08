#pragma once

// Purpose: automate destruction of vulkan objects without using vulkan.hpp
// Additional goal: seamless integration with vulkan.h structures
// Note: huge inspiration on `vulkan cookbook` repo idea ---
// https://github.com/PacktPublishing/Vulkan-Cookbook/blob/master/Library/Common%20Files/VulkanDestroyer.h

#include <utility>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace engine::core {

// Parental object
template <typename VkType> struct VkDestroyable {
private:
  VkType object = VK_NULL_HANDLE;

public:
  VkDestroyable() = default;
  VkDestroyable(VkType object) : object(object) {}

  VkDestroyable(const VkDestroyable &other) = delete;
  VkDestroyable &operator=(const VkDestroyable &other) = delete;

  VkDestroyable(VkDestroyable &&other) noexcept { *this = std::move(other); }

  VkDestroyable &operator=(VkDestroyable &&other) noexcept {
    if (this != std::addressof(other)) {
      this->~VkDestroyable();
      std::swap(object, other.object);
    }
    return *this;
  }

  VkType get_underlying() const { return object; }

  operator VkType() const { return object; }
  VkType *operator&() { return &object; }             // NOLINT
  const VkType *operator&() const { return &object; } // NOLINT

  ~VkDestroyable()
    requires std::is_same_v<VkType, VkInstance>
  {
    if (object != VK_NULL_HANDLE) {
      vkDestroyInstance(object, nullptr);
    }
  }

  ~VkDestroyable()
    requires std::is_same_v<VkType, VkDevice>
  {
    if (object != VK_NULL_HANDLE) {
      vkDestroyDevice(object, nullptr);
    }
  }
};

template <typename VkParentT, typename VkObjectT>
using VkDestroyFunctionT = void (*)(VkParentT, VkObjectT,
                                    const VkAllocationCallbacks *);

template <typename VkObjectT, typename VkParentT,
          VkDestroyFunctionT<VkParentT, VkObjectT> destroy_function>
struct VkDestroyableObjectWrapper {
  VkObjectT object = VK_NULL_HANDLE;
  VkParentT parent = VK_NULL_HANDLE;
};

// NOLINTBEGIN
#define ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkType, Destroyer)      \
  using VkType##Wrapper =                                                      \
      VkDestroyableObjectWrapper<VkType, VkInstance, Destroyer>

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);

ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkDebugUtilsMessengerEXT,
                                               DestroyDebugUtilsMessengerEXT);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkSurfaceKHR,
                                               vkDestroySurfaceKHR);

#undef ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC

#define ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkType, Destroyer)      \
  using VkType##Wrapper =                                                      \
      VkDestroyableObjectWrapper<VkType, VkDevice, Destroyer>

ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkSwapchainKHR,
                                               vkDestroySwapchainKHR);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkImage, vkDestroyImage);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkImageView, vkDestroyImageView);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkShaderModule,
                                               vkDestroyShaderModule);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkPipelineLayout,
                                               vkDestroyPipelineLayout);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkRenderPass,
                                               vkDestroyRenderPass);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkFramebuffer,
                                               vkDestroyFramebuffer);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkPipeline, vkDestroyPipeline);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkCommandPool,
                                               vkDestroyCommandPool);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkFence, vkDestroyFence);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkSemaphore, vkDestroySemaphore);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkBuffer, vkDestroyBuffer);
ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC(VkDeviceMemory, vkFreeMemory);

#undef ENGINE_CORE_VK_DESTROYABLE_OBJECT_WRAPPER_SPEC
// NOLINTEND

template <typename T, typename U, VkDestroyFunctionT<U, T> destroyer>
struct VkDestroyable<VkDestroyableObjectWrapper<T, U, destroyer>> {
private:
  using VkType = T;
  using WrappedT = VkDestroyableObjectWrapper<T, U, destroyer>;

  WrappedT wrapped;

public:
  VkDestroyable() = default;

  VkDestroyable(T object, U parent) : wrapped(object, parent) {};

  VkDestroyable(const VkDestroyable &other) = delete;
  VkDestroyable &operator=(const VkDestroyable &other) = delete;

  VkDestroyable(VkDestroyable &&other) noexcept { *this = std::move(other); }

  VkDestroyable &operator=(VkDestroyable &&other) noexcept {
    if (this != std::addressof(other)) {
      std::swap(wrapped, other.wrapped);
    }
    return *this;
  }

  ~VkDestroyable() {
    if (destroyer != nullptr && wrapped.parent != VK_NULL_HANDLE &&
        wrapped.object != VK_NULL_HANDLE) {
      destroyer(wrapped.parent, wrapped.object, nullptr);
    }
  }

  VkType get_underlying() const { return wrapped.object; }

  operator VkType() const { return wrapped.object; }

  VkType *operator&() { return &wrapped.object; }             // NOLINT
  const VkType *operator&() const { return &wrapped.object; } // NOLINT
};

} // namespace engine::core
