#pragma once

// Purpose: automate destruction of vulkan objects without using vulkan.hpp
// Additional goal: seamless integration with vulkan.h structures
// Note: huge inspiration on `vulkan cookbook` repo idea ---
// https://github.com/PacktPublishing/Vulkan-Cookbook/blob/master/Library/Common%20Files/VulkanDestroyer.h

#include <memory>
#include <utility>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace engine::core {

template <typename VkType> void destroy_vulkan_object(VkType);

template <> inline void destroy_vulkan_object<VkInstance>(VkInstance instance) {
  vkDestroyInstance(instance, nullptr);
}

template <> inline void destroy_vulkan_object<VkDevice>(VkDevice device) {
  vkDestroyDevice(device, nullptr);
}

template <typename VkType> struct VkDestroyable {
  VkType object = VK_NULL_HANDLE; // NOLINT

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

  ~VkDestroyable() {
    if (object != VK_NULL_HANDLE) {
      destroy_vulkan_object(object);
    }
  }
};

template <typename VkParent, typename VkChild>
using VkDestroyFunctionT = void (*)(VkParent, VkChild,
                                    const VkAllocationCallbacks *);

template <typename VkParent, typename VkChild>
void destroy_vulkan_object(VkParent parent, VkChild child,
                           VkDestroyFunctionT<VkParent, VkChild> destroyer) {
  destroyer(parent, child, nullptr);
}

template <typename VkChild, typename VkParent> struct VkDestroyableResource {
  VkChild object = VK_NULL_HANDLE;
  VkParent parent = VK_NULL_HANDLE;
  VkDestroyFunctionT<VkParent, VkChild> destroy_function = nullptr;
};

// NOLINTBEGIN
#define VK_DESTROYABLE_RESOURCE_SPEC(VkType, Destroyer)                        \
  using VkType##Wrapper = VkDestroyableResource<VkType, VkInstance>

VK_DESTROYABLE_RESOURCE_SPEC(VkDebugUtilsMessengerEXT,
                             vkDestroyDebugUtilsMessengerEXT);
VK_DESTROYABLE_RESOURCE_SPEC(VkSurfaceKHR, vkDestroySurfaceKHR);

#undef VK_DESTROYABLE_RESOURCE_SPEC

#define VK_DESTROYABLE_RESOURCE_SPEC(VkType, Destroyer)                        \
  using VkType##Wrapper = VkDestroyableResource<VkType, VkDevice>

VK_DESTROYABLE_RESOURCE_SPEC(VkSwapchainKHR, vkDestroySwapchainKHR);
VK_DESTROYABLE_RESOURCE_SPEC(VkImage, vkDestroyImage);
VK_DESTROYABLE_RESOURCE_SPEC(VkImageView, vkDestroyImageView);
VK_DESTROYABLE_RESOURCE_SPEC(VkShaderModule, vkDestroyShaderModule);
VK_DESTROYABLE_RESOURCE_SPEC(VkPipelineLayout, vkDestroyPipelineLayout);
VK_DESTROYABLE_RESOURCE_SPEC(VkRenderPass, vkDestroyRenderPass);
VK_DESTROYABLE_RESOURCE_SPEC(VkFramebuffer, vkDestroyFramebuffer);
VK_DESTROYABLE_RESOURCE_SPEC(VkPipeline, vkDestroyPipeline);
VK_DESTROYABLE_RESOURCE_SPEC(VkCommandPool, vkDestroyCommandPool);
VK_DESTROYABLE_RESOURCE_SPEC(VkFence, vkDestroyFence);
VK_DESTROYABLE_RESOURCE_SPEC(VkSemaphore, vkDestroySemaphore);

#undef VK_DESTROYABLE_RESOURCE_SPEC
// NOLINTEND

template <typename T, typename U>
struct VkDestroyable<VkDestroyableResource<T, U>> {
  using VkType = T;
  using WrappedT = VkDestroyableResource<T, U>;

  WrappedT wrapped; // NOLINT

  VkDestroyable() = default;

  VkDestroyable(T object, U parent, VkDestroyFunctionT<U, T> destroyer)
      : wrapped(object, parent, destroyer) {};

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
    if (wrapped.destroy_function != nullptr &&
        wrapped.parent != VK_NULL_HANDLE && wrapped.object != VK_NULL_HANDLE) {
      destroy_vulkan_object(wrapped.parent, wrapped.object,
                            wrapped.destroy_function);
    }
  }

  VkType get_underlying() const { return wrapped.object; }

  operator VkType() const { return wrapped.object; }

  VkType *operator&() { return &wrapped.object; }             // NOLINT
  const VkType *operator&() const { return &wrapped.object; } // NOLINT
};

} // namespace engine::core
