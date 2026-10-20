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
	void CreateSurface(GLFWwindow* window);
	void CreateSwapchain();
	bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
	void CreateImageViews();
	void CreateGraphicsPipeline();
	void CreateCommandPool();
	void CreateCommandBuffer();
	void recordCommandBuffer(uint32_t imageIndex);
	void transition_image_layout(
		uint32_t                imageIndex,
		vk::ImageLayout         old_layout,
		vk::ImageLayout         new_layout,
		vk::AccessFlags2        src_access_mask,
		vk::AccessFlags2        dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask);
	vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);
	[[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
	void DrawFrame();
	void CreateSyncObjects();
	vk::raii::Device const& GetDevice() const
	{
		return device;
	}

private:
	GLFWwindow* window = nullptr;
	vk::raii::Context context;
	vk::raii::Instance instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device device = nullptr;
	uint32_t queueIndex = ~0;
	vk::raii::Queue graphicsQueue = nullptr;
	vk::raii::SwapchainKHR swapChain = nullptr;
	std::vector<vk::Image> swapChainImages;
	vk::SurfaceFormatKHR swapChainSurfaceFormat;
	vk::Extent2D swapChainExtent;
	std::vector<vk::raii::ImageView> swapChainImageViews;

	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline graphicsPipeline = nullptr;
	vk::raii::CommandPool commandPool = nullptr;
	vk::raii::CommandBuffer commandBuffer = nullptr;

	vk::raii::Semaphore presentCompletSemaphore = nullptr;
	vk::raii::Semaphore renderFinishedSemaphore = nullptr;
	vk::raii::Fence drawFence = nullptr;

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

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

