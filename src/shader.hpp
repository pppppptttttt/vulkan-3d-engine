#pragma once

#include "engine_exceptions.hpp"
#include "meta.hpp"
#include "vulkan_destroyable.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace engine::core {

class Shader {
public:
  enum class Stage : std::uint8_t {
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    TESSELATION_CONTROL,
    TESSELATION_EVAL,
    COMPUTE
  };

private:
  static std::vector<char> read_file(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    std::vector<char> buffer(static_cast<std::size_t>(file.tellg()));
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    return buffer;
  }

  inline static const std::filesystem::path PATH_TO_SHADERS =
      PATH_TO_BINARIES / "compiled_shaders/";

  VkDestroyable<VkShaderModuleWrapper> m_module;

public:
  Shader(VkDevice device, const std::filesystem::path &relative_path) {
    const std::vector<char> code = read_file(PATH_TO_SHADERS / relative_path);

    const VkShaderModuleCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = code.size(),
        // NOLINTNEXTLINE
        .pCode = reinterpret_cast<const unsigned *>(code.data())};
    if (vkCreateShaderModule(device, &create_info, nullptr, &m_module) !=
        VK_SUCCESS) {
      throw exceptions::ShaderModuleCreationError{};
    }
    m_module.wrapped.parent = device;
    m_module.wrapped.destroy_function = vkDestroyShaderModule;
  }

  [[nodiscard]] VkShaderModule get_module() const { return m_module; }
};

} // namespace engine::core
