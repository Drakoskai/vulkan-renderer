#pragma once

#include <vulkan/vulkan.h>
#include "VkCom.h"

namespace Vulkan {

#define VK_FLAGS_NONE 0
#define DEFAULT_FENCE_TIMEOUT 100000000000

	std::string VkResultToString(VkResult result);
}
