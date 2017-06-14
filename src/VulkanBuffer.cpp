#include "VulkanBuffer.h"
#include <stdexcept>
#include "VulkanUtil.h"
#include "VulkanRenderSystem.h"

namespace Vulkan {
	VulkanBuffer::VulkanBuffer()
		: size_(0), alignment_(0), memoryPropertyFlags_(0), usageFlags_(0), data_(nullptr) {}

	VulkanBuffer::VulkanBuffer(VkMemoryPropertyFlags memoryPropertyFlags, VkBufferUsageFlags usageFlags)
		: size_(0), alignment_(0), memoryPropertyFlags_(memoryPropertyFlags), usageFlags_(usageFlags), data_(nullptr) {}

	VulkanBuffer::~VulkanBuffer() {}

	void VulkanBuffer::SetMemoryPropertyFlags(VkMemoryPropertyFlags memoryPropertyFlags) {
		memoryPropertyFlags_ = memoryPropertyFlags;
	}

	void VulkanBuffer::SetBufferUsageFlags(VkBufferUsageFlags usageFlags) {
		usageFlags_ = usageFlags;
	}

	VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset) {
		return vkMapMemory(VulkanRenderSystem::Device, bufferMemory_, offset, size, 0, &data_);
	}

	void VulkanBuffer::Unmap() {
		if (data_) {
			vkUnmapMemory(VulkanRenderSystem::Device, bufferMemory_);
			data_ = nullptr;
		}
	}

	VkResult VulkanBuffer::Bind(VkDeviceSize offset = 0) const {
		return vkBindBufferMemory(VulkanRenderSystem::Device, buffer_, bufferMemory_, offset);
	}

	void VulkanBuffer::CreateDescriptor(VkDeviceSize size, VkDeviceSize offset) {
		descriptor_.offset = offset;
		descriptor_.buffer = buffer_;
		descriptor_.range = size;
	}

	void VulkanBuffer::Copy(void const* data, VkDeviceSize size) const {		
		memcpy(data_, data, size);
	}

	void VulkanBuffer::CopyBuffer(const VkCom<VkBuffer>& src, VkDeviceSize size) {
		VulkanRenderSystem::CopyBuffer(src, buffer_, size);
	}

	void VulkanBuffer::CreateBuffer() {
		buffer_ = { VulkanRenderSystem::Device, vkDestroyBuffer };
		bufferMemory_ = { VulkanRenderSystem::Device, vkFreeMemory };

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size_;
		bufferInfo.usage = usageFlags_;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(VulkanRenderSystem::Device, &bufferInfo, nullptr, buffer_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanRenderSystem::Device, buffer_, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags_);

		if (vkAllocateMemory(VulkanRenderSystem::Device, &allocInfo, nullptr, bufferMemory_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}
		Bind();
		created = true;
	}

	const VkCom<VkBuffer>&  VulkanBuffer::Buffer() const {
		return buffer_;
	}
}
