cmake_minimum_required(VERSION 3.31)
project(3d-engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST_LIBRARY OFF)
add_subdirectory(third-party/SDL)

add_subdirectory(third-party/glm)

if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

# Compile shaders
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/compiled_shaders)

file(GLOB SHADERS ${SHADER_SOURCE_DIR}/*.glsl)

add_custom_command(
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  get_filename_component(EXTENSION ${source} EXT)
  string(SUBSTRING ${EXTENSION} 1 4 STAGE)
  add_custom_command(
    COMMAND
      ${glslc_executable} -fshader-stage=${STAGE}
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

# Compile project
include_directories(src/)
include_directories(third-party/)

file (GLOB_RECURSE CPP_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${CPP_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan SDL3::SDL3-static glm::glm)

# if directory with binaries located not in `./bin/`, you can specify
# it's location by passing `-DCUSTOM_PATH_TO_BINARIES=/path/to/bin`
# to cmake
if (DEFINED CUSTOM_PATH_TO_BINARIES)
  add_definitions(-DCUSTOM_PATH_TO_BINARIES="${CUSTOM_PATH_TO_BINARIES}")
  unset(CUSTOM_PATH_TO_BINARIES CACHE)
  unset(-DCUSTOM_PATH_TO_BINARIES CACHE)
endif ()
