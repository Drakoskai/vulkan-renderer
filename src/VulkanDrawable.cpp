#include "VulkanDrawable.h"
#include "VulkanUtil.h"
#include "VulkanRenderer.h"

namespace Vulkan {

	VulkanDrawable::VulkanDrawable() : pPipeline_(nullptr), pRenderer_(nullptr), mNumIndices_(0) {}

	VulkanDrawable::VulkanDrawable(VulkanRenderer* renderer) : pPipeline_(nullptr), pRenderer_(renderer), mNumIndices_(0) {
		vertexBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		vertexBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		indexBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		indexBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		uniformStagingBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformStagingBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		uniformBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		descriptorPool_ = { pRenderer_->device_, vkDestroyDescriptorPool };
	}

	VulkanDrawable::~VulkanDrawable() {}

	void VulkanDrawable::SetRenderDevice(VulkanRenderer* renderer) {
		pRenderer_ = renderer;
		vertexBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		vertexBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		indexBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		indexBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		uniformStagingBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformStagingBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		uniformBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		descriptorPool_ = { pRenderer_->device_, vkDestroyDescriptorPool };	
	}

	void VulkanDrawable::Generate(const std::vector<VertexPTN>& vertices, const std::vector<uint32_t>& indices, const Material& material) {
		if (!pRenderer_) {
			throw std::runtime_error("Vulkan Drawable not intialized!");
		}
		material_ = material;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vertices[0].GetAttributeDescriptions();
		std::vector<VkVertexInputBindingDescription> bindingDescriptions{ vertices[0].GetBindingDescription() };

		mNumIndices_ = static_cast<uint32_t>(indices.size());
		CreateVertexBuffer(vertices);
		CreateIndexBuffer(indices);
		pPipeline_ = pRenderer_->GetPipeline(0L);

		VertexBufferInfo vertexInfo = {
			attributeDescriptions,
			bindingDescriptions
		};

		pPipeline_->CreatePipeline(vertexInfo, material.shader);

		CreateUniformBuffer();
		CreateDescriptorPool();
		CreateDescriptorSet();
	}

	void VulkanDrawable::RecordDrawCommand(const VkCommandBuffer& commandBuffer) const {
		if (mNumIndices_ == 0 || !pPipeline_) { return; }

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline_->pipeline);

		VkBuffer vertexBuffers[] = { vertexBuffer_ };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline_->pipelineLayout, 0, 1, &descriptorSet_, 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, mNumIndices_, 1, 0, 0, 0);
	}

	void VulkanDrawable::CreateVertexBuffer(const std::vector<VertexPTN>& vertices) {
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

	void VulkanDrawable::CreateIndexBuffer(const std::vector<uint32_t>& indices) {
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

	void VulkanDrawable::CreateUniformBuffer() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer_, uniformStagingBufferMemory_);
		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer_, uniformBufferMemory_);
	}

	void VulkanDrawable::CreateDescriptorPool() {
		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(pRenderer_->device_, &poolInfo, nullptr, descriptorPool_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanDrawable::CreateDescriptorSet() {
		VkDescriptorSetLayout layouts[] = { pPipeline_->descriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool_;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		if (vkAllocateDescriptorSets(pRenderer_->device_, &allocInfo, &descriptorSet_) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer_;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSet_;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		VulkanTexture* diffuseTexture = pRenderer_->GetTexture(material_.diffuseTexture);
		if (diffuseTexture) {
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSet_;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &diffuseTexture->imageInfo_;
		}

		vkUpdateDescriptorSets(pRenderer_->device_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
