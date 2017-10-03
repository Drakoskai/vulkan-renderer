#include "VulkanBufferStaging.h"
#include "VulkanUtil.h"
#include <sstream>
#include "VulkanInit.h"

namespace Vulkan {

	BufferStaging::BufferStaging() : mMaxBufferSize(0), mCurrentBuffer(0), pMappedData(VK_NULL_HANDLE), hMemory(VK_NULL_HANDLE),
		hCommandPool(VK_NULL_HANDLE) {}

	BufferStaging::~BufferStaging() {}

	void BufferStaging::Init() {
		mMaxBufferSize = 64 * 1024 * 1024;
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = mMaxBufferSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		for (int i = 0; i < NumberOfFrameBuffers; ++i) {
			mBuffers[i].mOffset = 0;
			const VkResult result = vkCreateBuffer(context.device, &bufferCreateInfo, nullptr, &mBuffers[i].hBuffer);
			if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				throw std::runtime_error("failed to create staging mBuffer!");
			}
		}

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(context.device, mBuffers[0].hBuffer, &memoryRequirements);
		const VkDeviceSize alignMod = memoryRequirements.size % memoryRequirements.alignment;
		const VkDeviceSize alignedSize = alignMod == 0 ? memoryRequirements.size : memoryRequirements.size + memoryRequirements.alignment - alignMod;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(context.device, mBuffers[0].hBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = alignedSize * NumberOfFrameBuffers;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VkResult res = vkAllocateMemory(context.device, &allocInfo, nullptr, &hMemory);
		VkCheckOrThrow(res, "failed to allocate staging mBuffer memory!");

		res = vkMapMemory(context.device, hMemory, 0, alignedSize * NumberOfFrameBuffers, 0, reinterpret_cast<void **>(&pMappedData));
		VkCheckOrThrow(res, "failed to map staging mBuffer memory!");

		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = context.graphicsFamilyIdx;

		res = vkCreateCommandPool(context.device, &commandPoolCreateInfo, nullptr, &hCommandPool);
		VkCheckOrThrow(res, "failed to create staging command pool!");

		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = hCommandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		for (int i = 0; i < NumberOfFrameBuffers; ++i) {
			res = vkAllocateCommandBuffers(context.device, &commandBufferAllocateInfo, &mBuffers[i].hCommandBuffer);
			VkCheckOrThrow(res, "failed to allocate staging command buffers!");

			res = vkCreateFence(context.device, &fenceCreateInfo, NULL, &mBuffers[i].hFence);
			VkCheckOrThrow(res, "failed to create staging fence!");

			vkBeginCommandBuffer(mBuffers[i].hCommandBuffer, &commandBufferBeginInfo);
			mBuffers[i].pData = static_cast<uint8_t *>(pMappedData) + i * alignedSize;
		}
	}

	void BufferStaging::Shutdown() {
		vkUnmapMemory(context.device, hMemory);
		hMemory = VK_NULL_HANDLE;
		pMappedData = nullptr;

		for (int i = 0; i < NumberOfFrameBuffers; ++i) {
			vkDestroyFence(context.device, mBuffers[i].hFence, nullptr);
			vkDestroyBuffer(context.device, mBuffers[i].hBuffer, nullptr);
			vkFreeCommandBuffers(context.device, hCommandPool, 1, &mBuffers[i].hCommandBuffer);
		}
		memset(mBuffers, 0, sizeof(mBuffers));

		mMaxBufferSize = 0;
		mCurrentBuffer = 0;
	}

	uint8_t* BufferStaging::Stage(const uint32_t size, const uint32_t alignment, VkCommandBuffer& commandBuffer, VkBuffer& buffer, uint32_t& bufferOffset) {
		if (size > mMaxBufferSize) {
			std::ostringstream stringStream;
			stringStream << "Can't allocate %d MB in gpu transfer mBuffer", static_cast<uint32_t>(size / 1024 / 1024);
			throw std::runtime_error(stringStream.str());
		}
		StagingBuffer * staging = &mBuffers[mCurrentBuffer];
		const VkDeviceSize alignMod = staging->mOffset % alignment;
		const VkDeviceSize stagingOffset = staging->mOffset;
		staging->mOffset = stagingOffset % alignment == 0 ? stagingOffset : stagingOffset + alignment - alignMod;
		if (staging->mOffset + size >= mMaxBufferSize && !staging->mSubmitted) {
			Flush();
		}
		staging = &mBuffers[mCurrentBuffer];
		if (staging->mSubmitted) {
			WaitFor(*staging);
		}
		commandBuffer = staging->hCommandBuffer;
		buffer = staging->hBuffer;
		bufferOffset = static_cast<uint32_t>(staging->mOffset);

		uint8_t * data = staging->pData + staging->mOffset;
		staging->mOffset += size;

		return data;
	}

	void BufferStaging::Flush() {
		StagingBuffer & stage = mBuffers[mCurrentBuffer];
		if (stage.mSubmitted || stage.mOffset == 0) {
			return;
		}

		VkMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
		vkCmdPipelineBarrier(
			stage.hCommandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0, 1, &barrier, 0, NULL, 0, NULL);

		vkEndCommandBuffer(stage.hCommandBuffer);

		VkMappedMemoryRange memoryRange = {};
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = hMemory;
		memoryRange.size = VK_WHOLE_SIZE;
		vkFlushMappedMemoryRanges(context.device, 1, &memoryRange);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &stage.hCommandBuffer;

		vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, stage.hFence);

		stage.mSubmitted = true;

		mCurrentBuffer = (mCurrentBuffer + 1) % NumberOfFrameBuffers;
	}

	void BufferStaging::WaitFor(StagingBuffer& stage) {
		if (stage.mSubmitted == false) {
			return;
		}

		VkResult res = vkWaitForFences(context.device, 1, &stage.hFence, VK_TRUE, UINT64_MAX);
		VkCheckOrThrow(res, "failed to wait for staging fence!");

		res = vkResetFences(context.device, 1, &stage.hFence);
		VkCheckOrThrow(res, "failed to reset staging fence!");

		stage.mOffset = 0;
		stage.mSubmitted = false;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(stage.hCommandBuffer, &commandBufferBeginInfo);
	}
}
