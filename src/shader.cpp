#include "shader.hpp"
#include "engine_exceptions.hpp" // for ShaderModuleCreationError

namespace engine::core {

Shader::Shader(VkDevice device, const std::filesystem::path &relative_path) {
  const std::vector<char> code = read_file(PATH_TO_SHADERS / relative_path);

  const VkShaderModuleCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = code.size(),
      // NOLINTNEXTLINE
      .pCode = reinterpret_cast<const unsigned *>(code.data())};
  VkShaderModule module = VK_NULL_HANDLE;
  if (vkCreateShaderModule(device, &create_info, nullptr, &module) !=
      VK_SUCCESS) {
    throw exceptions::ShaderModuleCreationError{};
  }
  m_module = {module, device};
}

} // namespace engine::core
