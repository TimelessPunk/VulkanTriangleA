#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_core.h>
#include <GLFW/GLFW/glfw3.h>

class Renderer
{
public:
	void CreateInstance();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapchain();
	bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
	void CreateImageViews();
	void CreateGraphicsPipeline();

private:
	GLFWwindow* _window = nullptr;
	vk::raii::Context context;
	vk::raii::Instance instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device device = nullptr;
	vk::raii::Queue graphicsQueue = nullptr;
	vk::raii::SwapchainKHR swapChain = nullptr;
	std::vector<vk::Image> swapChainImages;
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	vk::Extent2D swapChainExtent;
	std::vector<vk::raii::ImageView> swapChainImageViews;
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vk::raii::PipelineLayout pipelineLayout = nullptr;


	std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

	std::vector<char const*> GetRequiredExtensions();
	std::vector<char const*> GetRequiredLayers();

	void SetupDebugMessenger();

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT, 
		vk::DebugUtilsMessageTypeFlagsEXT, 
		const vk::DebugUtilsMessengerCallbackDataEXT*,
		void*);
};

