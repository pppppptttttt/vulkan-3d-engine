#pragma once

#include <array>                // for array
#include <filesystem>           // for path
#include <string_view>          // for string_view
#include <vulkan/vulkan_core.h> // for VK_KHR_SWAPCHAIN_EXTENSION_NAME

namespace engine {

static constexpr std::string_view APPLICATION_NAME = "Vulkan 3D engine";

#ifndef CUSTOM_PATH_TO_BINARIES
inline static const std::filesystem::path PATH_TO_BINARIES = "../../bin/";
#else
inline static const std::filesystem::path PATH_TO_BINARIES =
    CUSTOM_PATH_TO_BINARIES;
#undef CUSTOM_PATH_TO_BINARIES
#endif

namespace core {

#ifndef NDEBUG
static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#else
static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif

static constexpr std::array<const char *, 1> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};

static constexpr std::array<const char *, 1> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};
} // namespace core

} // namespace engine
