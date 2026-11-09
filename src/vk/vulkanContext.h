#ifndef MYVULKAN_CONTEXT
#define MYVULKAN_CONTEXT

#include <vulkan/vulkan_raii.hpp>

class MyWindow;

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class VulkanContext{
    public:
        VulkanContext(const MyWindow& window);

        const vk::raii::Instance& getInstance() const;
        const vk::raii::SurfaceKHR& getSurface() const;
        const vk::raii::PhysicalDevice& getPhysicalDevice() const;
        const vk::raii::Device& getDevice() const;
        const vk::raii::Queue& getQueue() const;
        const uint32_t getQueueIndex() const;

    private:
        vk::raii::Context                    context;
        vk::raii::Instance                   instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT     debugMessenger = nullptr;
        vk::raii::SurfaceKHR                 surface = nullptr;
        
        std::vector<const char *> requiredInstanceExtensions;
        std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

        uint32_t                             queueIndex     = ~0;
        vk::raii::Queue                      queue          = nullptr;
        vk::raii::PhysicalDevice             physicalDevice = nullptr;
        vk::raii::Device                     device = nullptr;

        void createInstance();
        void setupDebugMessenger();
        void createSurface(const MyWindow& window);
        void pickPhysicalDevice();
        void createLogicalDevice();
        std::vector<const char *> getRequiredInstanceExtensions(const MyWindow& window);

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, 
                                                            vk::DebugUtilsMessageTypeFlagsEXT type,
                                                            const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, 
                                                            void *);

        bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);
};

#endif // MYVULKAN CONTEXT