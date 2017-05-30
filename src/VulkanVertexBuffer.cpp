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

	void VulkanVertexBuffer::AddGeometry(const SubMesh& submesh, VulkanPipeline* pipeline) {
		std::pair<SubMesh, VulkanPipeline*> pair;
		pair.first = submesh;
		pair.second = pipeline;
		geometries_.push_back(pair);
	}

	void VulkanVertexBuffer::Generate(VulkanDrawable* drawable) {
		uint32_t offset = 0;
		uint32_t idxoffset = 0;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		for (auto group : geometries_) {
			VertexBufferSection section;
			section.bindingDescription = group.first.vertices[0].GetBindingDescription();
			section.attributeDescriptions = group.first.vertices[0].GetAttributeDescriptions();
			section.buffer = this->vertexBuffer_;
			section.materialId = group.first.material.HashCode();
			section.offset = offset;
			section.idxoffset = idxoffset;
			section.pipeline = group.second;
			section.size = static_cast<uint32_t>(group.first.indices.size());

			for (size_t i = 0; i < group.first.vertices.size(); i++) {
				vertices.push_back(group.first.vertices[i]);
			}

			for (size_t i = 0; i < group.first.indices.size(); i++) {
				indices.push_back(group.first.indices[i]);
			}
			idxoffset = idxoffset + static_cast<uint32_t>(indices.size());
			offset = offset + static_cast<uint32_t>(vertices.size());

			section.descriptorSet = drawable->CreateDescriptorSet(group.second);
			drawable->AllocateDescriptorSet(section.descriptorSet, section.materialId);

			vertexBufferSections_.push_back(section);
		}
		geometries_.clear();
		CreateVertexBuffer(vertices);
		CreateIndexBuffer(indices);
	}

	void VulkanVertexBuffer::CreateVertexBuffer(const std::vector<Vertex>& vertices) {
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
