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
    App();

    void run();

    ~App();

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

    void createCommandBuffer();
    
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





};

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
		if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
		}

		return vk::False;
}

static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities){
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
  {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats){
  assert(!availableFormats.empty());
  const auto formatIt = std::ranges::find_if(
      availableFormats,
      [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}


static vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes){
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
  return std::ranges::any_of(availablePresentModes,
                              [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
              vk::PresentModeKHR::eMailbox :
              vk::PresentModeKHR::eFifo;
}

static std::vector<char> readFile(const std::string &filename){
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }
  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();
  return buffer;
}


#endif //MYAPP_SPLAT