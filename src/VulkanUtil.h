#pragma once

#include <vulkan/vulkan.h>
#include "VkCom.h"

namespace Vulkan {

	struct VulkanContext {
		VkInstance* instance;
		VkPhysicalDevice* physicalDevice;
		VkCom<VkDevice>* device;
		VkSwapchainKHR* swapChain;
		VkCommandPool* commandPool;
	};

#define VK_FLAGS_NONE 0
#define DEFAULT_FENCE_TIMEOUT 100000000000

	std::string VkResultToString(VkResult result);

}
