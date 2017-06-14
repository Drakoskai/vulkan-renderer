#ifndef VULKAN_UTL_H__
#define VULKAN_UTL_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanTypes.h"

namespace Vulkan {

#define VK_FLAGS_NONE 0
#define DEFAULT_FENCE_TIMEOUT 100000000000
	std::string VkResultToString(VkResult result);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	bool HasStencilComponent(VkFormat format);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
}
#endif
