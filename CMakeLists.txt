cmake_minimum_required(VERSION 3.25)
project(Splat-Viewer-Vulkan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(Vulkan REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(tinyply CONFIG REQUIRED)

# Source Files
add_executable(Splat-Viewer-Vulkan
    src/main.cpp
    src/app/app.cpp
    
)

target_include_directories(Splat-Viewer-Vulkan PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Definitions
target_compile_definitions(Splat-Viewer-Vulkan PRIVATE
    VK_NO_PROTOTYPES
    VOLK_IMPLEMENTATION
    VMA_IMPLEMENTATION
    VK_USE_PLATFORM_WIN32_KHR
    VULKAN_HPP_NO_CONSTRUCTORS
    GLFW_INCLUDE_VULKAN
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders/"
    ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)

# Link Libraries
target_link_libraries(Splat-Viewer-Vulkan PRIVATE
    Vulkan::Headers
    volk::volk_headers
    glfw
    glm::glm
    GPUOpen::VulkanMemoryAllocator
    tinyply
)

# GLSL -> SPIR-V Compilation
find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/Bin")
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found! Ensure the LunarG Vulkan SDK (1.4+) is installed.")
endif()

set(SHADER_SOURCES
)

set(SPV_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SPV_OUTPUT_DIR})

foreach(GLSL_FILE ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    set(SPV_FILE "${SPV_OUTPUT_DIR}/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${GLSLC_EXECUTABLE} --target-env=vulkan1.4 -I${CMAKE_CURRENT_SOURCE_DIR}/shaders -O ${GLSL_FILE} -o ${SPV_FILE}
        DEPENDS ${GLSL_FILE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/common.glsl"
        COMMENT "Compiling Vulkan 1.4 shader ${FILE_NAME} -> ${FILE_NAME}.spv"
        VERBATIM
    )
    list(APPEND SPV_BINARY_FILES ${SPV_FILE})
endforeach()

if(SPV_BINARY_FILES)
    add_custom_target(CompileShaders ALL DEPENDS ${SPV_BINARY_FILES})
    add_dependencies(Splat-Viewer-Vulkan CompileShaders)
endif()