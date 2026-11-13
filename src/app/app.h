#ifndef MYAPP_SPLAT
#define MYAPP_SPLAT

#include "app/myWindow.h"
#include <stdint.h>
#include <vector>
#include <memory>
#include "vk/vulkanContext.h"
#include "vk/mySwapchain.h"
#include "vk/myPipeline.h"

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

    std::unique_ptr<MySwapchain> mySwapchain = nullptr;

    std::unique_ptr<MyPipeline> myPipeline = nullptr;

    vk::raii::CommandPool                commandPool      = nullptr;
	  std::vector<vk::raii::CommandBuffer> commandBuffers;

    std::vector<vk::raii::Semaphore>     presentCompleteSemaphores;
	  std::vector<vk::raii::Semaphore>     renderFinishedSemaphores;
	  std::vector<vk::raii::Fence>         inFlightFences;
    uint32_t                             frameIndex = 0;

    void initVulkan();

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

    void recreateSwapChain();

};

#endif //MYAPP_SPLAT