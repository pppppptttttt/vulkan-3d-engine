#pragma once

#include "meta.hpp"               // for PATH_TO_BINARIES
#include "vulkan_destroyable.hpp" // for VkDestroyable, VkShaderModuleWrapper
#include <cstddef>                // for size_t
#include <cstdint>                // for uint8_t
#include <filesystem>             // for path, operator/
#include <fstream>                // for basic_ifstream, operator|, basic_ios
#include <vector>                 // for vector
#include <vulkan/vulkan_core.h>   // for VkDevice, VkShaderModule

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
  Shader(VkDevice device, const std::filesystem::path &relative_path);

  [[nodiscard]] VkShaderModule get_module() const { return m_module; }
};

} // namespace engine::core
