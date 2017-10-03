#include "VulkanBuffer.h"
#include "VulkanUtil.h"
#include "VulkanBufferStaging.h"
#include <stdexcept>
#include "VulkanAllocator.h"
#include <assert.h>
#include "VulkanInit.h"

namespace Vulkan {
	VulkanBuffer::VulkanBuffer() : hBuffer(VK_NULL_HANDLE), mSize(0), mOffset(0), mUsage() {}

	VulkanBuffer::~VulkanBuffer() {}

	void* VertexBuffer::Map() {
		void * buffer = mAllocation.data + GetOffset();
		SetMapped();
		if (buffer == nullptr) {
			throw std::runtime_error("failed to map vertex buffer!");
		}

		return buffer;
	}

	void VertexBuffer::Unmap() {
		SetUnmapped();
	}

	void VertexBuffer::ClearWithoutFreeing() {
		mSize = 0;
		mOffset = OwnerBit;
		hBuffer = VK_NULL_HANDLE;
		mAllocation.deviceMemory = VK_NULL_HANDLE;
	}

	VertexBuffer::VertexBuffer() {}

	VertexBuffer::~VertexBuffer() { FreeBufferObject(); }

	bool VertexBuffer::AllocateBufferObject(const void* data, int allocSize, BufferUsage usage) {
		mSize = allocSize;
		mUsage = usage;

		const int numBytes = GetAllocatedSize();

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.size = numBytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (usage == Static) {
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		VkResult ret = vkCreateBuffer(context.device, &bufferCreateInfo, nullptr, &hBuffer);
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(context.device, hBuffer, &memoryRequirements);

		const VulkanMemoryUsage memUsage = usage == Static ? Gpu : CpuToGpu;

		mAllocation = Allocator.Allocate(
			memoryRequirements.size,
			memoryRequirements.alignment,
			memoryRequirements.memoryTypeBits,
			memUsage,
			VulkanAllocationType::Buffer);

		ret = vkBindBufferMemory(context.device, hBuffer, mAllocation.deviceMemory, mAllocation.offset);
		const bool allocationFailed = ret != VK_SUCCESS ? true : false;
		if (data != nullptr) {
			Update(data, allocSize);
		}

		return !allocationFailed;
	}

	void VertexBuffer::FreeBufferObject() {
		if (IsMapped()) {
			Unmap();
		}
		if (IsOwner() == false) {
			ClearWithoutFreeing();
			return;
		}

		if (hBuffer == VK_NULL_HANDLE) {
			return;
		}

		if (hBuffer != VK_NULL_HANDLE) {
			Allocator.Free(mAllocation);
			vkDestroyBuffer(context.device, hBuffer, nullptr);
			hBuffer = VK_NULL_HANDLE;
			mAllocation = VulkanAllocation();
		}
		ClearWithoutFreeing();
	}

	void VertexBuffer::Reference(const VertexBuffer& other) {
		assert(IsMapped() == false);
		assert(other.GetSize() > 0);

		FreeBufferObject();
		mSize = other.GetOffset();
		mOffset = other.GetOffset();
		mUsage = other.mUsage;
		hBuffer = other.hBuffer;
		mAllocation = other.mAllocation;
		assert(IsOwner() == false);
	}

	void VertexBuffer::Reference(const VertexBuffer& other, int refOffset, int refSize) {
		assert(IsMapped() == false);
		assert(refOffset >= 0);
		assert(refSize >= 0);
		assert(refOffset + refSize <= other.GetSize());

		FreeBufferObject();
		mSize = refSize;
		mOffset = other.GetOffset() + refOffset;
		mUsage = other.mUsage;
		hBuffer = other.hBuffer;
		mAllocation = other.mAllocation;
		assert(IsOwner() == false);
	}


	void VertexBuffer::Update(const void* data, int size, int offset) const {
		VkBuffer stageBuffer = VK_NULL_HANDLE;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		uint32_t stageOffset = 0;
		uint8_t* stagingData = Staging.Stage(size, 1, commandBuffer, stageBuffer, stageOffset);

		memcpy(stagingData, data, size);
		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = static_cast<VkDeviceSize>(stageOffset);
		bufferCopy.dstOffset = static_cast<VkDeviceSize>(mOffset + offset);
		bufferCopy.size = static_cast<VkDeviceSize>(size);

		vkCmdCopyBuffer(commandBuffer, stageBuffer, hBuffer, 1, &bufferCopy);
	}

	IndexBuffer::IndexBuffer() {}

	IndexBuffer::~IndexBuffer() { FreeBufferObject(); }

	bool IndexBuffer::AllocateBufferObject(const void* data, int allocSize, BufferUsage usage) {
		mSize = allocSize;
		mUsage = usage;
		const int numBytes = GetAllocatedSize();

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.size = numBytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (usage == Static) {
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		VkResult ret = vkCreateBuffer(context.device, &bufferCreateInfo, nullptr, &hBuffer);
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(context.device, hBuffer, &memoryRequirements);

		const VulkanMemoryUsage memUsage = usage == Static ? Gpu : CpuToGpu;

		mAllocation = Allocator.Allocate(
			memoryRequirements.size,
			memoryRequirements.alignment,
			memoryRequirements.memoryTypeBits,
			memUsage,
			VulkanAllocationType::Buffer);

		ret = vkBindBufferMemory(context.device, hBuffer, mAllocation.deviceMemory, mAllocation.offset);
		const bool allocationFailed = ret != VK_SUCCESS ? true : false;
		if (data != nullptr) {
			Update(data, allocSize);
		}

		return !allocationFailed;
	}

	void IndexBuffer::FreeBufferObject() {
		if (IsMapped()) {
			Unmap();
		}
		if (IsOwner() == false) {
			ClearWithoutFreeing();
			return;
		}

		if (hBuffer == VK_NULL_HANDLE) {
			return;
		}

		if (hBuffer != VK_NULL_HANDLE) {
			Allocator.Free(mAllocation);
			vkDestroyBuffer(context.device, hBuffer, nullptr);
			hBuffer = VK_NULL_HANDLE;
			mAllocation = VulkanAllocation();
		}
		ClearWithoutFreeing();
	}

	void IndexBuffer::Reference(const IndexBuffer& other) {
		assert(IsMapped() == false);
		assert(other.GetSize() > 0);

		FreeBufferObject();
		mSize = other.GetOffset();
		mOffset = other.GetOffset();
		mUsage = other.mUsage;
		hBuffer = other.hBuffer;
		mAllocation = other.mAllocation;
		assert(IsOwner() == false);
	}

	void IndexBuffer::Reference(const IndexBuffer& other, int refOffset, int refSize) {
		assert(IsMapped() == false);
		assert(refOffset >= 0);
		assert(refSize >= 0);
		assert(refOffset + refSize <= other.GetSize());

		FreeBufferObject();
		mSize = refSize;
		mOffset = other.GetOffset() + refOffset;
		mUsage = other.mUsage;
		hBuffer = other.hBuffer;
		mAllocation = other.mAllocation;
		assert(IsOwner() == false);
	}

	void IndexBuffer::Update(const void* data, int size, int offset) const {
		VkBuffer stageBuffer = VK_NULL_HANDLE;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		uint32_t stageOffset = 0;
		uint8_t* stagingData = Staging.Stage(size, 1, commandBuffer, stageBuffer, stageOffset);

		memcpy(stagingData, data, size);
		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = static_cast<VkDeviceSize>(stageOffset);
		bufferCopy.dstOffset = static_cast<VkDeviceSize>(mOffset + offset);
		bufferCopy.size = static_cast<VkDeviceSize>(size);

		vkCmdCopyBuffer(commandBuffer, stageBuffer, hBuffer, 1, &bufferCopy);
	}

	void* IndexBuffer::Map() {
		void * buffer = mAllocation.data + GetOffset();
		SetMapped();
		if (buffer == nullptr) {
			throw std::runtime_error("failed to map index buffer!");
		}

		return buffer;
	}

	void IndexBuffer::Unmap() { SetUnmapped(); }

	void IndexBuffer::ClearWithoutFreeing() {
		mSize = 0;
		mOffset = OwnerBit;
		hBuffer = VK_NULL_HANDLE;
		mAllocation.deviceMemory = VK_NULL_HANDLE;
	}

	UniformBuffer::UniformBuffer() {
		mUsage = Dynamic;
		SetUnmapped();
	}

	UniformBuffer::~UniformBuffer() { ClearWithoutFreeing(); }

	bool UniformBuffer::AllocateBufferObject(const void* data, int allocSize, BufferUsage usage) {
		mSize = allocSize;
		mUsage = usage;
		const int numBytes = GetAllocatedSize();

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = NULL;
		bufferCreateInfo.size = numBytes;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (usage == Static) {
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		VkCheckOrThrow(vkCreateBuffer(context.device, &bufferCreateInfo, nullptr, &hBuffer), "failed to create uniform buffer!");

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(context.device, hBuffer, &memoryRequirements);

		const VulkanMemoryUsage memUsage = usage == Static ? Gpu : CpuToGpu;

		mAllocation = Allocator.Allocate(
			memoryRequirements.size,
			memoryRequirements.alignment,
			memoryRequirements.memoryTypeBits,
			memUsage,
			VulkanAllocationType::Buffer);

		VkResult res = vkBindBufferMemory(context.device, hBuffer, mAllocation.deviceMemory, mAllocation.offset);
		const bool allocationFailed = res != VK_SUCCESS ? true : false;
		if (data != nullptr) {
			Update(data, allocSize);
		}

		return !allocationFailed;
	}

	void UniformBuffer::FreeBufferObject() {
		if (IsMapped()) {
			Unmap();
		}
		if (IsOwner() == false) {
			ClearWithoutFreeing();
			return;
		}

		if (hBuffer == VK_NULL_HANDLE) {
			return;
		}

		if (hBuffer != VK_NULL_HANDLE) {
			Allocator.Free(mAllocation);
			vkDestroyBuffer(context.device, hBuffer, nullptr);
			hBuffer = VK_NULL_HANDLE;
			mAllocation = VulkanAllocation();
		}
		ClearWithoutFreeing();
	}

	void UniformBuffer::Reference(const UniformBuffer& other) {
		assert(IsMapped() == false);
		assert(other.GetSize() > 0);

		FreeBufferObject();
		mSize = other.GetOffset();
		mOffset = other.GetOffset();
		mUsage = other.mUsage;
		hBuffer = other.hBuffer;
		mAllocation = other.mAllocation;
		assert(IsOwner() == false);
	}

	void UniformBuffer::Reference(const UniformBuffer& other, int refOffset, int refSize) {
		assert(IsMapped() == false);
		assert(refOffset >= 0);
		assert(refSize >= 0);
		assert(refOffset + refSize <= other.GetSize());

		FreeBufferObject();
		mSize = refSize;
		mOffset = other.GetOffset() + refOffset;
		mUsage = other.mUsage;
		hBuffer = other.hBuffer;
		mAllocation = other.mAllocation;
		assert(IsOwner() == false);
	}

	void UniformBuffer::Update(const void* data, int size, int offset) const {
		VkBuffer stageBuffer = VK_NULL_HANDLE;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		uint32_t stageOffset = 0;
		uint8_t* stagingData = Staging.Stage(size, 1, commandBuffer, stageBuffer, stageOffset);

		memcpy(stagingData, data, size);
		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = static_cast<VkDeviceSize>(stageOffset);
		bufferCopy.dstOffset = static_cast<VkDeviceSize>(mOffset + offset);
		bufferCopy.size = static_cast<VkDeviceSize>(size);

		vkCmdCopyBuffer(commandBuffer, stageBuffer, hBuffer, 1, &bufferCopy);
	}

	void* UniformBuffer::Map() {
		void* buffer = mAllocation.data + GetOffset();
		SetMapped();
		if (buffer == nullptr) {
			throw std::runtime_error("failed to map index buffer!");
		}

		return buffer;
	}

	void UniformBuffer::Unmap() { SetUnmapped(); }

	void UniformBuffer::ClearWithoutFreeing() {
		mSize = 0;
		mOffset = OwnerBit;
		hBuffer = VK_NULL_HANDLE;
		mAllocation.deviceMemory = VK_NULL_HANDLE;
	}
}
