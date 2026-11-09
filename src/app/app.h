#ifndef MYAPP_SPLAT
#define MYAPP_SPLAT

#include "app/myWindow.h"
#include <stdint.h>
#include <vector>
#include "vk/vulkanContext.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t W_WIDTH = 1280;
constexpr uint32_t W_HEIGHT = 720;

class App {

  public:
    App();

    void run();


  private:
    std::unique_ptr<MyWindow> myWindow = std::make_unique<MyWindow>(W_WIDTH,W_HEIGHT);
    
    std::unique_ptr<VulkanContext> VKContext = nullptr;

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

    void initVulkan();

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

    static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);

    static vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);

    static std::vector<char> readFile(const std::string &filename);

};

#endif //MYAPP_SPLAT