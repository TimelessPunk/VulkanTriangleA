#include "Renderer.h"
#include "Debug.h"
#include <iostream>

//#include <print>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/GLFW/glfw3native.h>

void Renderer::CreateInstance()
{
	constexpr vk::ApplicationInfo app_info
	{
		.pApplicationName = "Vulkan triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Gaming Campus Triangle",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	std::vector requiredExtensions = GetRequiredExtensions();

	auto extensionProperties = context.enumerateInstanceExtensionProperties();

	for (uint32_t i = 0; i < requiredExtensions.size(); ++i)
	{
		if (std::ranges::none_of(extensionProperties,
			[glfwExtension = requiredExtensions[i]](auto const& extensionProperty)
			{
				return strcmp(extensionProperty.extensionName, glfwExtension);
			}
		)) 
		{
			throw std::runtime_error("Required extension not supported: " + std::string(requiredExtensions[i]));
		}
	}

	std::vector requiredLayers = GetRequiredLayers();
	auto layerProperties = context.enumerateInstanceLayerProperties();

	auto predicate_layer = [&layerProperties](auto const& requiredLayer)
	{
		return std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty)
		{
			return strcmp(layerProperty.layerName, requiredLayer) == 0;
		});
	};

	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, predicate_layer);

	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Message layer not suppported: " + std::string(*unsupportedLayerIt));
	}

	vk::InstanceCreateInfo instance_create_info
	{
		.pApplicationInfo = &app_info,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data(),
	};

	instance = vk::raii::Instance(context, instance_create_info);

	SetupDebugMessenger();
}

void Renderer::PickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
	auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) { return isDeviceSuitable(physicalDevice); });
	if (devIter == physicalDevices.end())
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	physicalDevice = *devIter;
}

void Renderer::CreateLogicalDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	uint32_t queueIndex = ~0;
	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && 
			physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
		{
			queueIndex = qfpIndex;
			break;
		}
	}
	if (queueIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{},
			{.shaderDrawParameters = true},
			{.dynamicRendering = true},
			{.extendedDynamicState = true} 
		};

	float queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
	vk::DeviceCreateInfo deviceCreateInfo{
	.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
	.queueCreateInfoCount = 1,
	.pQueueCreateInfos = &deviceQueueCreateInfo,
	.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
	.ppEnabledExtensionNames = requiredDeviceExtension.data()
	};

	device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
}

void Renderer::CreateSurface()
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
	{
		throw std::runtime_error("failed to create window surface !");
	}
	surface = vk::raii::SurfaceKHR(instance, _surface);
}

std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice)
{
	bool supportsVulkan1_4 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion14;

	auto queueFamilies = physicalDevice.getQueueFamilyProperties();
	bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp)
		{ return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions = std::ranges::all_of(requiredDeviceExtension,
		[&availableDeviceExtensions](auto const& requiredDeviceExtension)
		{
			return std::ranges::any_of(availableDeviceExtensions,
				[requiredDeviceExtension](auto const& availableDeviceExtension)
				{
					return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
		});

	auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

	bool supportRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
		features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
		features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	return supportsVulkan1_4 && supportsGraphics && supportsAllRequiredExtensions && supportRequiredFeatures;
}

std::vector<char const*> Renderer::GetRequiredExtensions()
{
	uint32_t glfwExtensionsCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

	std::vector<char const*> applicationExtension = { vk::EXTDebugUtilsExtensionName };

	std::vector requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

	requiredExtensions.append_range(applicationExtension);

	return requiredExtensions;
}

std::vector<char const*> Renderer::GetRequiredLayers()
{
	const std::vector validationLayers = { "VK_LAYER_KHRONOS_validation" };

	std::vector<char const*> requiredLayers = {};

	if (Debug::EnableDebug)
		requiredLayers.append_range(validationLayers);

	return requiredLayers;
}

void Renderer::SetupDebugMessenger()
{
	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

	vk::DebugUtilsMessageTypeFlagsEXT typeFlags(
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	
	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo
	{
		.messageSeverity = severityFlags,
		.messageType = typeFlags,
		.pfnUserCallback = &DebugCallback,
	};

	debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo);
}

vk::Bool32 Renderer::DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* data, void*)
{
	std::cout << "Validation Layer : Type [" << vk::to_string(type) << "] - Message : " << data->pMessage << std::endl;

	/*std::println(std::cout, "Validation Layer : Type [{}]", vk::to_string(type));
	std::println(std::cout, "Message : {}", data->pMessage);*/

	return vk::False;
}