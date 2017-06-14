#ifndef VULKAN_BUFFER_H_
#define VULKAN_BUFFER_H_

#include <vulkan/vulkan.h>
#include "VkCom.h"

namespace Vulkan {
	class VulkanBuffer {
	public:
		VulkanBuffer();
		VulkanBuffer(VkMemoryPropertyFlags memoryPropertyFlags, VkBufferUsageFlags usageFlags);
		virtual ~VulkanBuffer();
		virtual void SetSize(VkDeviceSize size) { size_ = size; }
		virtual void SetAlignment(VkDeviceSize alignment) { alignment_ = alignment; }
		virtual void SetMemoryPropertyFlags(VkMemoryPropertyFlags memoryPropertyFlags);
		virtual void SetBufferUsageFlags(VkBufferUsageFlags usageFlags);
		virtual VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		virtual void Unmap();
		virtual VkResult Bind(VkDeviceSize offset) const;
		virtual void CreateDescriptor(VkDeviceSize size, VkDeviceSize offset);
		virtual void Copy(void const* data, VkDeviceSize size) const;
		virtual void CopyBuffer(const VkCom<VkBuffer>& src, VkDeviceSize size);
		virtual void CreateBuffer();
		virtual const VkCom<VkBuffer>& Buffer() const;
	protected:		
		VkCom<VkDeviceMemory> bufferMemory_;
		VkCom<VkBuffer> buffer_;
		VkDescriptorBufferInfo descriptor_;
		VkDeviceSize size_;
		VkDeviceSize alignment_;
		VkMemoryPropertyFlags memoryPropertyFlags_;
		VkBufferUsageFlags usageFlags_;
		void* data_;
		bool created = false;
	};
}
#endif
