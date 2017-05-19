#include "VulkanVertexBuffer.h"
#include "VulkanRenderer.h"

namespace Vulkan {
	VulkanVertexBuffer::VulkanVertexBuffer() : pRenderer_(nullptr) {}
	VulkanVertexBuffer::~VulkanVertexBuffer() {}

	void VulkanVertexBuffer::SetRenderer(VulkanRenderer* renderer) {
		pRenderer_ = renderer;
		vertexBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		vertexBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		indexBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		indexBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
	}

	void VulkanVertexBuffer::Generate(const std::vector<VertexPTN>& vertices, const std::vector<uint32_t>& indices) {
		CreateVertexBuffer(vertices);
		CreateIndexBuffer(indices);
		attributeDescriptions_ = vertices[0].GetAttributeDescriptions();
		bindingDescriptions_ = { vertices[0].GetBindingDescription() };
	}

	void VulkanVertexBuffer::CreateVertexBuffer(const std::vector<VertexPTN>& vertices) {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		VkCom<VkBuffer> stagingBuffer{ pRenderer_->device_, vkDestroyBuffer };
		VkCom<VkDeviceMemory> stagingBufferMemory{ pRenderer_->device_, vkFreeMemory };
		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(pRenderer_->device_, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(pRenderer_->device_, stagingBufferMemory);

		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer_, vertexBufferMemory_);
		pRenderer_->CopyBuffer(stagingBuffer, vertexBuffer_, bufferSize);
	}

	void VulkanVertexBuffer::CreateIndexBuffer(const std::vector<uint32_t>& indices) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
		VkCom<VkBuffer> stagingBuffer{ pRenderer_->device_, vkDestroyBuffer };
		VkCom<VkDeviceMemory> stagingBufferMemory{ pRenderer_->device_, vkFreeMemory };
		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(pRenderer_->device_, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(pRenderer_->device_, stagingBufferMemory);

		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer_, indexBufferMemory_);
		pRenderer_->CopyBuffer(stagingBuffer, indexBuffer_, bufferSize);
	}
}
