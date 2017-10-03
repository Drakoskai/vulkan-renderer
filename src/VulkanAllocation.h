#ifndef VULKAN_ALLOCATION_H_
#define VULKAN_ALLOCATION_H_

#include <vulkan/vulkan.h>

namespace Vulkan {

	enum VulkanMemoryUsage {
		Unknown,
		Gpu,
		Cpu,
		CpuToGpu,
		GpuToCpu
	};

	uint32_t FindMemoryTypeIndex(const uint32_t memoryTypeBits, const VulkanMemoryUsage usage);

	enum VulkanAllocationType {
		Free,
		Buffer,
		Image,
		ImageLinear,
		ImageOptimal
	};

	class VulkanBlock;

	struct VulkanAllocation {
		VulkanBlock* block;
		uint32_t id;
		VkDeviceMemory deviceMemory;
		VkDeviceSize offset;
		VkDeviceSize size;
		uint8_t* data;
	};
}
#endif
