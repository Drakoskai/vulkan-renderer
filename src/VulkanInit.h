#ifndef VULKAN_INIT_H_
#define VULKAN_INIT_H_

#include "stdafx.h"
#include <vector>
#include <array>
#include "VulkanBufferStaging.h"

namespace Vulkan {

	struct GpuInfo {
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties physicalDeviceProps;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		VkPhysicalDeviceMemoryProperties memProperties;
		VkSurfaceCapabilitiesKHR surfaceCaps;
		std::vector<VkSurfaceFormatKHR>	surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;
		std::vector<VkQueueFamilyProperties> queueFamilies;
		std::vector<VkExtensionProperties> availableExtensions;
	};

	struct VulkanContext {
		uint64_t counter;
		uint32_t currentFrame;
		GpuInfo* gpu;
		std::vector<GpuInfo> gpus;

		VkDevice device;
		int graphicsFamilyIdx;
		int	presentFamilyIdx;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

		VkCommandPool commandPool;
		VkPipelineCache pipelineCache;
		VkRenderPass renderPass;
		VkCommandBuffer commandBuffer;
		std::array<VkCommandBuffer, NumberOfFrameBuffers> commandBuffers;
		std::array<VkFence, NumberOfFrameBuffers> commandBufferFences;
	};

	extern VulkanContext context;

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, uint32_t code, const char * layerPrefix, const char * msg, void * userData);
	void CreateDebugReportCallback(VkInstance instance);
	void DestroyDebugReportCallback(VkInstance instance);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	std::vector<const char*> GetRequiredExtensions();
	bool CheckDeviceExtensionSupport(GpuInfo& gpu, std::vector<const char*>& requiredExt);
	void CheckValidationLayerSupport();
	VkFormat ChooseSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkCommandBuffer BeginCommandBuffer();
	void EndCommandBuffer(VkCommandBuffer commandBuffer);
	void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory bufferMemory);
	void CopyImage(VkImage& srcImage, VkImage& dstImage, uint32_t width, uint32_t height);
	void CopyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);
}
#endif
