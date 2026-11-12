#include "vk/vulkanContext.h"
#include "app/myWindow.h"

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <iostream>

VulkanContext::VulkanContext(const MyWindow& window){
    requiredInstanceExtensions = getRequiredInstanceExtensions(window);
    createInstance();
    setupDebugMessenger();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
}

std::vector<const char *> VulkanContext::getRequiredInstanceExtensions(const MyWindow& window){
	uint32_t glfwExtensionCount = 0;
	auto     glfwExtensions     = window.getInstanceExtensions(glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

void VulkanContext::createSurface(const MyWindow& window){
    surface = window.createSurface(instance);
}

void VulkanContext::createInstance(){
    constexpr vk::ApplicationInfo appInfo{.pApplicationName   = "Splat Viewer",
		                                      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		                                      .pEngineName        = "No Engine",
		                                      .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
		                                      .apiVersion         = vk::ApiVersion14};

    // Get the required layers
    std::vector<char const *> requiredLayers;
    if (enableValidationLayers)
    {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties    = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
                                                    [&layerProperties](auto const &requiredLayer) {
                                                        return std::ranges::none_of(layerProperties,
                                                                                    [requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
                                                    });
    if (unsupportedLayerIt != requiredLayers.end())
    {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }


    // Check if the required extensions are supported by the Vulkan implementation.
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt =
        std::ranges::find_if(requiredInstanceExtensions,
                                [&extensionProperties](auto const &requiredExtension) {
                                    return std::ranges::none_of(extensionProperties,
                                                                [requiredExtension](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
                                });
    if (unsupportedPropertyIt != requiredInstanceExtensions.end())
    {
        throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
    }

    vk::InstanceCreateInfo createInfo{.pApplicationInfo        = &appInfo,
                                        .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
                                        .ppEnabledLayerNames     = requiredLayers.data(),
                                        .enabledExtensionCount   = static_cast<uint32_t>(requiredInstanceExtensions.size()),
                                        .ppEnabledExtensionNames = requiredInstanceExtensions.data()};
    instance = vk::raii::Instance(context, createInfo);
}

const vk::raii::Instance& VulkanContext::getInstance() const{
    if (instance == nullptr) {
        throw std::runtime_error("Vulkan instance is null!");
    }
    return instance;
}

const vk::raii::SurfaceKHR& VulkanContext::getSurface() const{
    if (surface == nullptr) {
        throw std::runtime_error("Surface is null!");
    }
    return surface;
}

const vk::raii::Device& VulkanContext::getDevice() const {
    if (device == nullptr){
     throw std::runtime_error("Logical Device is null!");
    }
    return device;
}

const vk::raii::PhysicalDevice& VulkanContext::getPhysicalDevice() const{
    if (physicalDevice == nullptr){
     throw std::runtime_error("Physical Device is null!");
    }
    return physicalDevice;
}

const vk::raii::Queue& VulkanContext::getQueue() const{
    return queue;
}

const uint32_t VulkanContext::getQueueIndex() const{
    return queueIndex;
}

void VulkanContext::setupDebugMessenger(){
    if (!enableValidationLayers)
		return;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
														vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,
																			.messageType     = messageTypeFlags,
																			.pfnUserCallback = &debugCallback};
	debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanContext::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
		if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
		}

		return vk::False;
}

void VulkanContext::pickPhysicalDevice(){
    std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
	auto const                            devIter         = std::ranges::find_if(physicalDevices, [&](auto const &physicalDevice) { return isDeviceSuitable(physicalDevice); });
	if (devIter == physicalDevices.end())
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	physicalDevice = *devIter;
}

bool VulkanContext::isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice){
	// Check if the physicalDevice supports the Vulkan 1.4 API version
	bool supportsVulkan1_4 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion14;

	// Check if any of the queue families support graphics operations
	auto queueFamilies    = physicalDevice.getQueueFamilyProperties();
	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const &qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

	// Check if all required physicalDevice extensions are available
	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(requiredDeviceExtension,
							[&availableDeviceExtensions](auto const &requiredDeviceExtension) {
								return std::ranges::any_of(availableDeviceExtensions,
															[requiredDeviceExtension](auto const &availableDeviceExtension) { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
							});

	// Check if the physicalDevice supports the required features
	auto features                 = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
																			vk::PhysicalDeviceVulkan11Features,
																			vk::PhysicalDeviceVulkan13Features,
																			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
									features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
									features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	// Return true if the physicalDevice meets all the criteria
	return supportsVulkan1_4 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

void VulkanContext::createLogicalDevice(){
    // find the index of the first queue family that supports graphics
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports both graphics and present
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
    {
      if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
          physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
      {
        // found a queue family that supports both graphics and present
        queueIndex = qfpIndex;
        break;
      }
    }
    if (queueIndex == ~0)
    {
      throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
    }
	// query for Vulkan 1.4 features
	vk::StructureChain<vk::PhysicalDeviceFeatures2,
						vk::PhysicalDeviceVulkan11Features,
						vk::PhysicalDeviceVulkan13Features,
						vk::PhysicalDeviceVulkan14Features,
						vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{},                                    // vk::PhysicalDeviceFeatures2
			{.shaderDrawParameters = true},        // vk::PhysicalDeviceVulkan11Features
			{.synchronization2 = true,             // vk::PhysicalDeviceVulkan13Features
			 .dynamicRendering = true},            
			{},                                    // vk::PhyscicalDeviceVulkan14Features
			{.extendedDynamicState = true}         // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		};

	// create a Device
	float                     queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
	vk::DeviceCreateInfo      deviceCreateInfo{.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
												.queueCreateInfoCount    = 1,
												.pQueueCreateInfos       = &deviceQueueCreateInfo,
												.enabledExtensionCount   = static_cast<uint32_t>(requiredDeviceExtension.size()),
												.ppEnabledExtensionNames = requiredDeviceExtension.data()};

	device        = vk::raii::Device(physicalDevice, deviceCreateInfo);
	queue  = vk::raii::Queue(device, queueIndex, 0);
}