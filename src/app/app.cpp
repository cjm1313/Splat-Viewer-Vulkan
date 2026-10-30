#include "app/app.h"

#include <stdexcept>
#include <volk.h>
#include <vma/vk_mem_alloc.h>


App::App(){
    if (!glfwInit()){
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // not using opengl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // no window resizing


    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "Splat Viewer", nullptr, nullptr);


    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
        glfwTerminate();
        
    }

    initVulkan();
}


void App::run(){

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); 
    }

}

void App::createInstance(){
    constexpr vk::ApplicationInfo appInfo{.pApplicationName   = "Splat Viewer",
                                          .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
                                          .pEngineName        = "No Engine",
                                          .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
                                          .apiVersion         = vk::ApiVersion14};

    // Get the required instance extensions from GLFW.
    uint32_t glfwExtensionCount = 0;
    auto     glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Check if the required GLFW extensions are supported by the Vulkan implementation.
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        if (std::ranges::none_of(extensionProperties,
                                    [glfwExtension = glfwExtensions[i]](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
        }
    }

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo        = &appInfo,
        .enabledExtensionCount   = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions};

    instance = vk::raii::Instance(context, createInfo);

}

void App::initVulkan(){
    createInstance();
}


App::~App(){
    glfwDestroyWindow(window);
    glfwTerminate();
}