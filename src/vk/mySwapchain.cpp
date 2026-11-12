#include "vk/mySwapchain.h"
#include "vk/vulkanContext.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <cassert>

MySwapchain::MySwapchain(const VulkanContext& ctx, uint32_t width, uint32_t height){
    auto& device = ctx.getDevice();
    auto& physicalDevice = ctx.getPhysicalDevice();
    auto& surface = ctx.getSurface();
    create(physicalDevice, device, surface, width, height);
}

void MySwapchain::create(const vk::raii::PhysicalDevice& physicalDevice,
                         const vk::raii::Device& device, 
                         const vk::raii::SurfaceKHR& surface,
                         uint32_t width,
                         uint32_t height) {
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
	swapChainExtent                                = chooseSwapExtent(surfaceCapabilities, width, height);
	uint32_t minImageCount                         = chooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
	swapChainSurfaceFormat                             = chooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
	vk::PresentModeKHR              presentMode           = chooseSwapPresentMode(availablePresentModes);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface          = *surface,
													.minImageCount    = minImageCount,
													.imageFormat      = swapChainSurfaceFormat.format,
													.imageColorSpace  = swapChainSurfaceFormat.colorSpace,
													.imageExtent      = swapChainExtent,
													.imageArrayLayers = 1,
													.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
													.imageSharingMode = vk::SharingMode::eExclusive,
													.preTransform     = surfaceCapabilities.currentTransform,
													.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
													.presentMode      = presentMode,
													.clipped          = true};

	swapChain       = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
	swapChainImages = swapChain.getImages();

    assert(swapChainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo{.viewType         = vk::ImageViewType::e2D,
												.format           = swapChainSurfaceFormat.format,
												.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

	for (auto &image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
}

const vk::raii::SwapchainKHR& MySwapchain::getSwapchain() const {
    if (swapChain == nullptr) {
        throw std::runtime_error("Swapchain is null!");
    }
    return swapChain;
}

const std::vector<vk::Image>& MySwapchain::getImages() const {
    return swapChainImages;
}

const vk::SurfaceFormatKHR& MySwapchain::getSurfaceFormat() const {
    return swapChainSurfaceFormat;
}

const vk::Extent2D& MySwapchain::getExtent() const {
    return swapChainExtent;
}

const std::vector<vk::raii::ImageView>& MySwapchain::getImageViews() const {
    return swapChainImageViews;
}

void MySwapchain::cleanup(){
    swapChainImageViews.clear();
    swapChain = nullptr;
}

void MySwapchain::recreate(const VulkanContext& ctx, uint32_t width, uint32_t height){
    auto& device = ctx.getDevice();
    auto& physicalDevice = ctx.getPhysicalDevice();
    auto& surface = ctx.getSurface();

	cleanup();
	create(physicalDevice, device, surface, width, height);
}

vk::Extent2D MySwapchain::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities, uint32_t width, uint32_t height){
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	
	return {
	    std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
	    std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
}

uint32_t MySwapchain::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities){
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
  {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

vk::SurfaceFormatKHR MySwapchain::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats){
  assert(!availableFormats.empty());
  const auto formatIt = std::ranges::find_if(
      availableFormats,
      [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR MySwapchain::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes){
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
  return std::ranges::any_of(availablePresentModes,
                              [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
              vk::PresentModeKHR::eMailbox :
              vk::PresentModeKHR::eFifo;
}