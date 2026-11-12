#ifndef MY_SWAPCHAIN_H
#define MY_SWAPCHAIN_H

#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <cstdint>

class VulkanContext;

class MySwapchain{
    public:
        MySwapchain(const VulkanContext& ctx, uint32_t width, uint32_t height);
        void recreate(const VulkanContext& ctx, uint32_t width, uint32_t height);

        const vk::raii::SwapchainKHR& getSwapchain() const;
        const std::vector<vk::Image>& getImages() const;
        const vk::SurfaceFormatKHR& getSurfaceFormat() const;
        const vk::Extent2D& getExtent() const;
        const std::vector<vk::raii::ImageView>& getImageViews() const;

    private:
        vk::raii::SwapchainKHR               swapChain      = nullptr;
        std::vector<vk::Image>               swapChainImages;
        vk::SurfaceFormatKHR                 swapChainSurfaceFormat;
        vk::Extent2D                         swapChainExtent;
        std::vector<vk::raii::ImageView>     swapChainImageViews;

        void create(const vk::raii::PhysicalDevice& physicalDevice,
                    const vk::raii::Device& device, 
                    const vk::raii::SurfaceKHR& surface,
                    uint32_t width,
                    uint32_t height);

        void cleanup();

        static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, uint32_t width, uint32_t height);

        static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR &surfaceCapabilities);

        static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

        static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
};

#endif // MY_SWAPCHAIN_H