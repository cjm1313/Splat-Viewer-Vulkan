#ifndef MYAPP_SPLAT
#define MYAPP_SPLAT

#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>
#include <stdint.h>
#include <vector>
#include <fstream>
#include <vulkan/vulkan_raii.hpp>

const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class App {

  public:

    void run();


  private:
    uint32_t W_WIDTH = 1280;
    uint32_t W_HEIGHT = 720;
    GLFWwindow* window = nullptr;

    

    vk::raii::Context                    context;
    vk::raii::Instance                   instance = nullptr;
    
    vk::raii::DebugUtilsMessengerEXT     debugMessenger = nullptr;
    
    vk::raii::SurfaceKHR                 surface = nullptr;
    
    uint32_t                             queueIndex     = ~0;
    vk::raii::Queue                      queue          = nullptr;
    vk::raii::PhysicalDevice             physicalDevice = nullptr;
    vk::raii::Device                     device = nullptr;
        

    vk::raii::SwapchainKHR               swapChain      = nullptr;
	  std::vector<vk::Image>               swapChainImages;
	  vk::SurfaceFormatKHR                 swapChainSurfaceFormat;
	  vk::Extent2D                         swapChainExtent;
	  std::vector<vk::raii::ImageView>     swapChainImageViews;

    vk::raii::PipelineLayout             pipelineLayout = nullptr;
    vk::raii::Pipeline                   graphicsPipeline = nullptr;
    vk::raii::CommandPool                commandPool      = nullptr;
	  std::vector<vk::raii::CommandBuffer> commandBuffers;

    std::vector<vk::raii::Semaphore>     presentCompleteSemaphores;
	  std::vector<vk::raii::Semaphore>     renderFinishedSemaphores;
	  std::vector<vk::raii::Fence>         inFlightFences;
    uint32_t                             frameIndex = 0;

    bool framebufferResized = false;
    std::vector<const char *> requiredDeviceExtension = {
	    vk::KHRSwapchainExtensionName};



    void initVulkan();

    void createInstance();

    void setupDebugMessenger();

    std::vector<const char *> getRequiredInstanceExtensions();

    bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSurface();

    void createSwapChain();

    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);

    void createImageViews();
    
    void createGraphicsPipeline();

    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;

    void createCommandBuffers();
    
    void createCommandPool();

    void createSyncObjects();

    void recordCommandBuffer(uint32_t imageIndex);

    void transition_image_layout(uint32_t                imageIndex,
                                 vk::ImageLayout         old_layout,
                                 vk::ImageLayout         new_layout,
                                 vk::AccessFlags2        src_access_mask,
                                 vk::AccessFlags2        dst_access_mask,
                                 vk::PipelineStageFlags2 src_stage_mask,
                                 vk::PipelineStageFlags2 dst_stage_mask);

    void drawFrame();

    void cleanupSwapChain();

    void recreateSwapChain();

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *);
    
    static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);

    static vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);

    static std::vector<char> readFile(const std::string &filename);

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);



};




#endif //MYAPP_SPLAT