#include "app/app.h"

#include <stdexcept>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include <fstream>
#include <iostream>


App::App(){
	

    initVulkan();
}

void App::run(){
	
    while (!myWindow->wantsToClose()) {
        myWindow->pollEvents();
		drawFrame();
    }

	auto& device = VKContext->getDevice();
	device.waitIdle();
}

void App::initVulkan(){
	{
		createInstance();

		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
	}
	{
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

}

void App::createInstance(){
	VKContext->setRequiredInstanceExtensions(getRequiredInstanceExtensions());
    VKContext->createInstance();
}



void App::setupDebugMessenger(){
	VKContext->setupDebugMessenger();
}

std::vector<const char *> App::getRequiredInstanceExtensions(){
	uint32_t glfwExtensionCount = 0;
	auto     glfwExtensions     = myWindow->getInstanceExtensions(glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}



void App::pickPhysicalDevice(){
	VKContext->pickPhysicalDevice();
}

void App::createLogicalDevice(){
	VKContext->createLogicalDevice(surface);
}

void App::createSurface(){
    surface = myWindow->createSurface(VKContext->getInstance());
}

void App::createSwapChain(){
	auto& physicalDevice = VKContext->getPhysicalDevice();
	auto& device = VKContext->getDevice();
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
	swapChainExtent                                = chooseSwapExtent(surfaceCapabilities);
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
}

vk::Extent2D App::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities){
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		int width, height;
		myWindow->getFramebufferSize(&width, &height);
		
		return {
		    std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		    std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
}

void App::createImageViews(){
	assert(swapChainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo{.viewType         = vk::ImageViewType::e2D,
												.format           = swapChainSurfaceFormat.format,
												.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

	auto& device = VKContext->getDevice();
	for (auto &image : swapChainImages)
	{
		imageViewCreateInfo.image = image;
		swapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
}

void App::createGraphicsPipeline() {
	vk::raii::ShaderModule shaderModuleVert = createShaderModule(readFile(std::string(SHADER_DIR) + "shader.vert.spv"));
	vk::raii::ShaderModule shaderModuleFrag = createShaderModule(readFile(std::string(SHADER_DIR) + "shader.frag.spv"));

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModuleVert, .pName = "main"};
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModuleFrag, .pName = "main"};
	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


	vk::PipelineVertexInputStateCreateInfo   vertexInputInfo;
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
	vk::PipelineViewportStateCreateInfo      viewportState{.viewportCount = 1, .scissorCount = 1};

	vk::PipelineRasterizationStateCreateInfo rasterizer{.depthClampEnable        = vk::False,
														.rasterizerDiscardEnable = vk::False,
														.polygonMode             = vk::PolygonMode::eFill,
														.cullMode                = vk::CullModeFlagBits::eBack,
														.frontFace               = vk::FrontFace::eClockwise,
														.depthBiasEnable         = vk::False,
														.lineWidth               = 1.0f};

	vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable    = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

	std::vector<vk::DynamicState>      dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};
	

	auto& device = VKContext->getDevice();
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0, .pushConstantRangeCount = 0};
	pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
		{.stageCount          = 2,
			.pStages             = shaderStages,
			.pVertexInputState   = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState      = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState   = &multisampling,
			.pColorBlendState    = &colorBlending,
			.pDynamicState       = &dynamicState,
			.layout              = pipelineLayout,
			.renderPass          = nullptr},
		{.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format}};

	graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

vk::raii::ShaderModule App::createShaderModule(const std::vector<char> &code) const {

	auto& device = VKContext->getDevice();
	vk::ShaderModuleCreateInfo createInfo{.codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t *>(code.data())};
	vk::raii::ShaderModule     shaderModule{device, createInfo};

	return shaderModule;
}

void App::createCommandPool(){
	auto& device = VKContext->getDevice();
	auto queueIndex = VKContext->getQueueIndex();
	vk::CommandPoolCreateInfo poolInfo{.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
										.queueFamilyIndex = queueIndex};
	commandPool = vk::raii::CommandPool(device, poolInfo);
}

void App::createCommandBuffers(){
	auto& device = VKContext->getDevice();
	vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
	commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}

void App::createSyncObjects(){
	assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());
	auto& device = VKContext->getDevice();
	for (size_t i = 0; i < swapChainImages.size(); i++){
		renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
		presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
		inFlightFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
	}
}


void App::recordCommandBuffer(uint32_t imageIndex){
	auto &commandBuffer = commandBuffers[frameIndex];
	commandBuffer.begin({});

	// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
	);
	vk::ClearColorValue colorValue = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
	vk::ClearValue clearValue(colorValue);
	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView   = swapChainImageViews[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp      = vk::AttachmentLoadOp::eClear,
		.storeOp     = vk::AttachmentStoreOp::eStore,
		.clearValue  = clearValue};
	vk::RenderingInfo renderingInfo = {
		.renderArea           = {.offset = {0, 0}, .extent = swapChainExtent},
		.layerCount           = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments    = &attachmentInfo};

	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
	commandBuffer.draw(3, 1, 0, 0);
	commandBuffer.endRendering();

	// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
	transition_image_layout(
		imageIndex,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		{},                                                        // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe                  // dstStage
	);
	commandBuffer.end();
}

void App::transition_image_layout(
	uint32_t                imageIndex,
	vk::ImageLayout         old_layout,
	vk::ImageLayout         new_layout,
	vk::AccessFlags2        src_access_mask,
	vk::AccessFlags2        dst_access_mask,
	vk::PipelineStageFlags2 src_stage_mask,
	vk::PipelineStageFlags2 dst_stage_mask){
		
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask        = src_stage_mask,
		.srcAccessMask       = src_access_mask,
		.dstStageMask        = dst_stage_mask,
		.dstAccessMask       = dst_access_mask,
		.oldLayout           = old_layout,
		.newLayout           = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image               = swapChainImages[imageIndex],
		.subresourceRange    = {
				.aspectMask     = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1}};
	vk::DependencyInfo dependency_info = {
		.dependencyFlags         = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers    = &barrier};
	commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
}


void App::drawFrame() {
    // Wait for the previous frame using this slot to finish
	auto& device = VKContext->getDevice();
	auto& queue = VKContext-> getQueue();
    auto fenceResult = device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("failed to wait for fence!");
    }

    // Acquire the next image from the swap chain (catch OutOfDateKHRError on resize)
    uint32_t imageIndex;
    try {
        auto [result, idx] = swapChain.acquireNextImage(
            UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapChain();
            return;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        imageIndex = idx;
    } catch (const vk::OutOfDateKHRError&) {
        recreateSwapChain();
        return;
    }

    // Only reset the fence AFTER successfully acquiring an image to prevent a deadlock if we return early above
    device.resetFences(*inFlightFences[frameIndex]);

    // Reset and record the command buffer for the acquired swap chain image
    commandBuffers[frameIndex].reset();
    recordCommandBuffer(imageIndex);

    // Submit the command buffer to the queue
    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
        .pWaitDstStageMask    = &waitDestinationStageMask,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &*commandBuffers[frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]
    };
    queue.submit(submitInfo, *inFlightFences[frameIndex]);

    // Present the rendered image (catch OutOfDateKHRError thrown by vk::raii::Queue::presentKHR)
    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
        .swapchainCount     = 1,
        .pSwapchains        = &*swapChain,
        .pImageIndices      = &imageIndex
    };

    try {
        auto result = queue.presentKHR(presentInfoKHR);
        if (result == vk::Result::eSuboptimalKHR || (myWindow->getFramebufferResized())) {
			myWindow->setFramebufferResized(false);
            recreateSwapChain();
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    } catch (const vk::OutOfDateKHRError&) {
        myWindow->setFramebufferResized(false);
        recreateSwapChain();
    }

    // Advance to the next frame in flight
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::cleanupSwapChain(){
    swapChainImageViews.clear();
    swapChain = nullptr;
}

void App::recreateSwapChain(){
    int width = 0, height = 0;
	myWindow->getFramebufferSize(&width, &height);
	while ((width == 0 || height == 0) && !(myWindow->wantsToClose()))
	{
		myWindow->getFramebufferSize(&width, &height);
		myWindow->waitEvents();
	}
	if (myWindow->wantsToClose())
	{
		return;
	}

	auto& device = VKContext->getDevice();
	device.waitIdle();

	cleanupSwapChain();
	createSwapChain();
	createImageViews();
}






uint32_t App::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities){
  auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
  {
    minImageCount = surfaceCapabilities.maxImageCount;
  }
  return minImageCount;
}

vk::SurfaceFormatKHR App::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats){
  assert(!availableFormats.empty());
  const auto formatIt = std::ranges::find_if(
      availableFormats,
      [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
  return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}


vk::PresentModeKHR App::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes){
  assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
  return std::ranges::any_of(availablePresentModes,
                              [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
              vk::PresentModeKHR::eMailbox :
              vk::PresentModeKHR::eFifo;
}

std::vector<char> App::readFile(const std::string &filename){
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



 
