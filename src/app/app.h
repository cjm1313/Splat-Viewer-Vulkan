#ifndef MYAPP_SPLAT
#define MYAPP_SPLAT

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif



class App {

  public:
    App();

    void run();

    ~App();

  private:
    uint32_t W_WIDTH = 1280;
    uint32_t W_HEIGHT = 720;
    GLFWwindow* window = nullptr;

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;

    std::vector<const char *> requiredDeviceExtension = {
	    vk::KHRSwapchainExtensionName};



    void initVulkan();

    void createInstance();

    void setupDebugMessenger();

    std::vector<const char *> getRequiredInstanceExtensions();

    bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);
    void pickPhysicalDevice();

    




};

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
		if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
		}

		return vk::False;
}

#endif //MYAPP_SPLAT