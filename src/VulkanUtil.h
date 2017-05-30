#ifndef VULKAN_UTL_H__
#define VULKAN_UTL_H__

#include <vulkan/vulkan.h>
#include <string>

namespace Vulkan {

#define VK_FLAGS_NONE 0
#define DEFAULT_FENCE_TIMEOUT 100000000000

	std::string VkResultToString(VkResult result);
}
#endif
