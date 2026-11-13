#include "app/app.h"

#include <stdexcept>
#include <limits>
#include <algorithm>
#include <iostream>


App::App(){
	VKContext = std::make_unique<VulkanContext>(*myWindow);

	int width = 0, height = 0;
	myWindow->getFramebufferSize(&width, &height);
	mySwapchain = std::make_unique<MySwapchain>(
		*VKContext,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	);

	auto pipelineConfig = PipelineConfigInfo::defaultConfig(mySwapchain->getSurfaceFormat().format);
	myPipeline = std::make_unique<MyPipeline>(*VKContext, pipelineConfig);

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
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
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
	const auto& swapChainImages = mySwapchain->getImages();
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
	const auto& swapChainImageViews = mySwapchain->getImageViews();
	const auto& swapChainExtent = mySwapchain->getExtent();

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
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *myPipeline->getPipeline());
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
		
	const auto& swapChainImages = mySwapchain->getImages();
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
	auto& queue = VKContext->getQueue();
	auto& swapChain = mySwapchain->getSwapchain();
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

	mySwapchain->recreate(
		*VKContext,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	);
}