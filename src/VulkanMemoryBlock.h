#ifndef VULKAN_MEMORY_BLOCK_H_
#define VULKAN_MEMORY_BLOCK_H_

#include <vulkan/vulkan.h>
#include "VulkanAllocation.h"

namespace Vulkan {
	class VulkanBlock {
		friend class VulkanAllocator;
	public:
		VulkanBlock(const uint32_t memoryTypeIndex, const VkDeviceSize size, VulkanMemoryUsage usage);
		~VulkanBlock();
		bool Init();
		void Shutdown();
		bool IsHostVisible() const { return mUsage != Gpu; }
		bool Allocate(const VkDeviceSize size, const VkDeviceSize align, const VkDeviceSize granularity, const VulkanAllocationType allocType, VulkanAllocation & allocation);
		void Free(VulkanAllocation & allocation);
	private:
		struct Chunk {
			uint32_t mId;
			VkDeviceSize mSize;
			VkDeviceSize mOffset;
			VulkanMemoryUsage mUsage;
			Chunk* pPrev;
			Chunk* pNext;
			VulkanAllocationType mType;
		};
		Chunk* pHead;
		uint32_t mNextBlockId;
		uint32_t mMemoryTypeIndex;
		VulkanMemoryUsage mUsage;
		VkDeviceMemory hDeviceMemory;
		VkDeviceSize mSize;
		VkDeviceSize mAllocated;
		uint8_t* pData;
	};
}
#endif
