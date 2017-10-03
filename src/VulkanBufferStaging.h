#ifndef VULKAN_BUFFER_STAGING_H
#define VULKAN_BUFFER_STAGING_H

#include <vulkan/vulkan.h>
#include "VulkanTypes.h"

namespace Vulkan {
	struct StagingBuffer {
		bool mSubmitted;
		VkCommandBuffer hCommandBuffer;
		VkBuffer hBuffer;
		VkFence hFence;
		VkDeviceSize mOffset;
		uint8_t* pData;
		StagingBuffer() : mSubmitted(false), hCommandBuffer(VK_NULL_HANDLE), hBuffer(VK_NULL_HANDLE), hFence(VK_NULL_HANDLE),
			mOffset(0), pData(nullptr) {}
	};

	class BufferStaging {
	public:
		BufferStaging();
		~BufferStaging();
		void Init();
		void Shutdown();
		uint8_t* Stage(const uint32_t size, const uint32_t alignment, VkCommandBuffer& commandBuffer, VkBuffer& buffer, uint32_t& bufferOffset);
		void Flush();
	private:
		static void WaitFor(StagingBuffer & stage);
		uint32_t mMaxBufferSize;
		uint32_t mCurrentBuffer;
		void* pMappedData;
		VkDeviceMemory hMemory;
		VkCommandPool hCommandPool;

		StagingBuffer mBuffers[NumberOfFrameBuffers];
	};

	static BufferStaging Staging;
}
#endif
