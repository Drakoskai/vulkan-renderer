#include "VulkanMemoryBlock.h"
#include "VulkanUtil.h"
#include "VulkanInit.h"

namespace Vulkan {
	VulkanBlock::VulkanBlock(const uint32_t memoryTypeIndex, const VkDeviceSize deviceSize, VulkanMemoryUsage usage) :
		pHead(nullptr), mNextBlockId(0), mMemoryTypeIndex(memoryTypeIndex), mUsage(usage), hDeviceMemory(VK_NULL_HANDLE), mSize(deviceSize), mAllocated(0), pData(nullptr) {}

	VulkanBlock::~VulkanBlock() { Shutdown(); }

	bool VulkanBlock::Init() {
		if (mMemoryTypeIndex == UINT64_MAX) {
			return false;
		}
		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = mSize;
		memoryAllocateInfo.memoryTypeIndex = mMemoryTypeIndex;

		if (vkAllocateMemory(context.device, &memoryAllocateInfo, nullptr, &hDeviceMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		if (hDeviceMemory == VK_NULL_HANDLE) {
			return false;
		}

		if (IsHostVisible()) {
			if (vkMapMemory(context.device, hDeviceMemory, 0, mSize, 0, reinterpret_cast<void**>(&pData)) != VK_SUCCESS) {
				throw std::runtime_error("failed to map to buffer memory!");
			}
		}

		pHead = new Chunk();
		pHead->mSize = mSize;
		pHead->mOffset = 0;
		pHead->pPrev = nullptr;
		pHead->pNext = nullptr;
		pHead->mType = VulkanAllocationType::Free;

		return true;
	}

	void VulkanBlock::Shutdown() {
		if (IsHostVisible()) {
			vkUnmapMemory(context.device, hDeviceMemory);
		}
		vkFreeMemory(context.device, hDeviceMemory, NULL);
		hDeviceMemory = VK_NULL_HANDLE;

		Chunk* current = pHead;
		while (true) {
			if (current->pNext == nullptr) {
				delete current;
				break;
			}
			Chunk* prev = current;
			current = current->pNext;
			delete prev;
		}

		pHead = nullptr;
	}
	/*
	=============
	IsOnSamePage

	Algorithm comes from the Vulkan 1.0.39 spec. "Buffer-Image Granularity"
	Also known as "Linear-Optimal Granularity"
	=============
	*/
	static bool IsOnSamePage(
		VkDeviceSize rAOffset, VkDeviceSize rASize,
		VkDeviceSize rBOffset, VkDeviceSize pageSize) {

		assert(rAOffset + rASize <= rBOffset && rASize > 0 && pageSize > 0);

		const VkDeviceSize rAEnd = rAOffset + rASize - 1;
		const VkDeviceSize rAEndPage = rAEnd & ~(pageSize - 1);
		const VkDeviceSize rBStart = rBOffset;
		const VkDeviceSize rBStartPage = rBStart & ~(pageSize - 1);

		return rAEndPage == rBStartPage;
	}
	/*
	=============
	HasGranularityConflict

	Check that allocation types obey buffer image granularity.
	=============
	*/
	static bool HasGranularityConflict(VulkanAllocationType type1, VulkanAllocationType type2) {
		if (type1 > type2) {
			SwapValues(type1, type2);
		}

		switch (type1) {
		case Free:
			return false;
		case Buffer:
			return	type2 == Image || type2 == ImageOptimal;
		case Image:
			return  type2 == Image || type2 == ImageLinear || type2 == ImageOptimal;
		case ImageLinear:
			return type2 == ImageOptimal;
		case ImageOptimal:
			return false;
		default:
			assert(false);
			return true;
		}
	}

	bool VulkanBlock::Allocate(const VkDeviceSize size, const VkDeviceSize align, const VkDeviceSize granularity, const VulkanAllocationType allocType, VulkanAllocation& allocation) {
		const VkDeviceSize freeSize = size - mAllocated;
		if (freeSize < size) {
			return false;
		}

		Chunk* current = nullptr;
		Chunk* bestFit = nullptr;
		Chunk* previous = nullptr;

		VkDeviceSize padding = 0;
		VkDeviceSize offset = 0;
		VkDeviceSize alignedSize = 0;

		for (current = pHead; current != nullptr; previous = current, current = current->pNext) {
			if (!current->mType == VulkanAllocationType::Free) {
				continue;
			}

			if (size > current->mSize) {
				continue;
			}

			offset = Align(current->mOffset, align);

			// Check for linear/optimal granularity conflict with previous allocation
			if (previous != nullptr && granularity > 1) {
				if (IsOnSamePage(previous->mOffset, previous->mSize, offset, granularity)) {
					if (HasGranularityConflict(previous->mType, allocType)) {
						offset = Align(offset, granularity);
					}
				}
			}

			padding = offset - current->mOffset;
			alignedSize = padding + size;

			if (alignedSize > current->mSize) {
				continue;
			}

			if (alignedSize + mAllocated >= mSize) {
				return false;
			}

			if (granularity > 1 && current->pNext != nullptr) {
				Chunk* next = current->pNext;
				if (IsOnSamePage(offset, size, next->mOffset, granularity)) {
					if (HasGranularityConflict(allocType, next->mType)) {
						continue;
					}
				}
			}

			bestFit = current;
			break;
		}

		if (bestFit == NULL) {
			return false;
		}

		if (bestFit->mSize > size) {
			Chunk * chunk = new Chunk();
			chunk->mId = mNextBlockId++;
			chunk->pPrev = bestFit;
			chunk->pNext = bestFit->pNext;
			bestFit->pNext = chunk;
			chunk->mSize = bestFit->mSize - alignedSize;
			chunk->mOffset = offset + size;
			chunk->mType = VulkanAllocationType::Free;
		}

		bestFit->mType = allocType;
		bestFit->mSize = size;

		mAllocated += alignedSize;

		allocation.size = bestFit->mSize;
		allocation.id = bestFit->mId;
		allocation.deviceMemory = hDeviceMemory;
		if (IsHostVisible()) {
			allocation.data = pData + offset;
		}
		allocation.offset = offset;
		allocation.block = this;

		return true;
	}

	void VulkanBlock::Free(VulkanAllocation& allocation) {
		Chunk* current;
		for (current = pHead; current != NULL; current = current->pNext) {
			if (current->mId == allocation.id) {
				break;
			}
		}

		if (current == nullptr) {
			return;
		}

		if (current->pPrev && current->pPrev->mType == VulkanAllocationType::Free) {
			Chunk* prev = current->pPrev;

			prev->pNext = current->pNext;
			if (current->pNext) {
				current->pNext->pPrev = prev;
			}

			prev->mSize += current->mSize;

			delete current;
			current = prev;
		}

		if (current->pNext && current->pNext->mType == VulkanAllocationType::Free) {
			Chunk* next = current->pNext;

			if (next->pNext) {
				next->pNext->pPrev = current;
			}

			current->pNext = next->pNext;

			current->mSize += next->mSize;

			delete next;
		}

		mAllocated -= allocation.size;
	}
}
