#include "VulkanAllocation.h"
#include <stdexcept>
#include "VulkanInit.h"

namespace Vulkan {
	uint32_t FindMemoryTypeIndex(const uint32_t memoryTypeBits, const VulkanMemoryUsage usage) {
		const VkPhysicalDeviceMemoryProperties & physicalMemoryProperties = context.gpu->memProperties;
		VkMemoryPropertyFlags required = 0;
		VkMemoryPropertyFlags preferred = 0;

		switch (usage) {
		case Gpu:
			preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		case Cpu:
			required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			break;
		case CpuToGpu:
			required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		case GpuToCpu:
			required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			preferred |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			break;
		default:
			throw std::runtime_error("Unknown memory usage!");
		}

		for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i) {
			if ((memoryTypeBits >> i & 1) == 0) {
				continue;
			}

			const VkMemoryPropertyFlags properties = physicalMemoryProperties.memoryTypes[i].propertyFlags;
			if ((properties & required) != required) {
				continue;
			}

			if ((properties & preferred) != preferred) {
				continue;
			}

			return i;
		}

		for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i) {
			if ((memoryTypeBits >> i & 1) == 0) {
				continue;
			}

			const VkMemoryPropertyFlags properties = physicalMemoryProperties.memoryTypes[i].propertyFlags;
			if ((properties & required) != required) {
				continue;
			}

			return i;
		}

		return UINT32_MAX;
	}
}
