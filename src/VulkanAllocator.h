#ifndef VULKAN_ALLOCATOR_H_
#define VULKAN_ALLOCATOR_H_

#include <array>
#include <vulkan/vulkan.h>
#include "VulkanTypes.h"
#include "VulkanAllocation.h"
#include "VulkanMemoryBlock.h"

namespace Vulkan {
	class VulkanAllocator {
	public:
		VulkanAllocator();
		~VulkanAllocator();
		void Init();
		void Shutdown();
		VulkanAllocation Allocate(const VkDeviceSize size, const VkDeviceSize align, const uint32_t memoryTypeBits, const VulkanMemoryUsage usage, const VulkanAllocationType allocType);
		void Free(const VulkanAllocation allocation);
		void EmptyGarbage();
	private:
		uint32_t mGarbageIndex;
		uint32_t mDeviceLocalMemoryMB;
		uint32_t mHostVisibleMemoryMB;
		VkDeviceSize mBufferImageGranularity;

		std::array<std::vector<VulkanBlock*>, VK_MAX_MEMORY_TYPES> mBlocks;
		std::vector<std::vector<VulkanAllocation>> mGarbage;
	};

	extern VulkanAllocator Allocator;
}
#endif
