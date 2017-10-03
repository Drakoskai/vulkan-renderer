#include "VulkanAllocator.h"
#include "VulkanRenderSystem.h"
#include "VulkanInit.h"

namespace Vulkan {
	VulkanAllocator::VulkanAllocator() : mGarbageIndex(0), mDeviceLocalMemoryMB(0), mHostVisibleMemoryMB(0), mBufferImageGranularity(0) {}

	VulkanAllocator::~VulkanAllocator() {}

	void VulkanAllocator::Init() {
		mDeviceLocalMemoryMB = 128 * 1024 * 1024;
		mHostVisibleMemoryMB = 64 * 1024 * 1024;
		mBufferImageGranularity = context.gpu->physicalDeviceProps.limits.bufferImageGranularity;
	}

	void VulkanAllocator::Shutdown() {
		EmptyGarbage();
		for (int i = 0; i < VK_MAX_MEMORY_TYPES; ++i) {
			std::vector<VulkanBlock*>& vulkanBlocks = mBlocks[i];
			vulkanBlocks.clear();
		}
	}

	VulkanAllocation VulkanAllocator::Allocate(const VkDeviceSize size, const VkDeviceSize align, const uint32_t memoryTypeBits, const VulkanMemoryUsage usage, const VulkanAllocationType allocType) {
		VulkanAllocation allocation;
		const uint32_t memoryTypeIndex = FindMemoryTypeIndex(memoryTypeBits, usage);
		if (memoryTypeIndex == UINT32_MAX) {
			throw std::runtime_error("Unable to find a memoryTypeIndex for mAllocation request.");
		}
		std::vector<VulkanBlock*>& vulkanBlocks = mBlocks[memoryTypeIndex];
		const auto numBlocks = mBlocks.size();
		for (auto i = 0; i < numBlocks; ++i) {
			if (vulkanBlocks.size() <= i) {
				continue;
			}
			VulkanBlock* block = vulkanBlocks[i];

			if (block && block->mMemoryTypeIndex != memoryTypeIndex) {
				continue;
			}

			if (block->Allocate(size, align, mBufferImageGranularity, allocType, allocation)) {
				return allocation;
			}
		}
		const VkDeviceSize blockSize = (usage == Gpu) ? mDeviceLocalMemoryMB : mHostVisibleMemoryMB;
		VulkanBlock* block = new VulkanBlock(memoryTypeIndex, blockSize, usage);
		if (block->Init()) {
			vulkanBlocks.push_back(block);
		} else {
			throw std::runtime_error("failed to allocate new memory block!");
		}

		block->Allocate(size, align, mBufferImageGranularity, allocType, allocation);

		return allocation;
	}

	void VulkanAllocator::Free(const VulkanAllocation allocation) {
		mGarbage[mGarbageIndex].push_back(allocation);
	}

	void VulkanAllocator::EmptyGarbage() {
		mGarbageIndex = (mGarbageIndex + 1) % NumberOfFrameBuffers;
		std::vector<VulkanAllocation> & g = mGarbage[mGarbageIndex];
		const auto numAllocations = mGarbage.size();
		for (auto i = 0; i < numAllocations; ++i) {
			VulkanAllocation allocation = g[i];
			allocation.block->Free(allocation);
			if (allocation.block->mAllocated == 0) {
				std::vector<VulkanBlock*> vkBlocks = mBlocks[allocation.block->mMemoryTypeIndex];
				vkBlocks.erase(remove(vkBlocks.begin(), vkBlocks.end(), allocation.block), vkBlocks.end());
				delete allocation.block;
				allocation.block = NULL;
			}
		}
		g.clear();
	}
}
