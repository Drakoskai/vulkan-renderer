#include "VulkanDrawable.h"
#include "VulkanUtil.h"
#include "VulkanRenderer.h"

namespace Vulkan {

	VulkanDrawable::VulkanDrawable() : pRenderer(nullptr), mNumIndices(0) {}

	VulkanDrawable::VulkanDrawable(VulkanRenderer* renderer) : pPipeline(nullptr), pRenderer(renderer), mNumIndices(0) {
		vertexBuffer = { pRenderer->device_, vkDestroyBuffer };
		vertexBufferMemory = { pRenderer->device_, vkFreeMemory };
		indexBuffer = { pRenderer->device_, vkDestroyBuffer };
		indexBufferMemory = { pRenderer->device_, vkFreeMemory };
		uniformStagingBuffer = { pRenderer->device_, vkDestroyBuffer };
		uniformStagingBufferMemory = { pRenderer->device_, vkFreeMemory };
		uniformBuffer = { pRenderer->device_, vkDestroyBuffer };
		uniformBufferMemory = { pRenderer->device_, vkFreeMemory };
		descriptorPool = { pRenderer->device_, vkDestroyDescriptorPool };
		descriptorSetLayout = { pRenderer->device_, vkDestroyDescriptorSetLayout };
		pPipeline = { pRenderer->device_, vkDestroyPipeline };
	}

	VulkanDrawable::~VulkanDrawable() {}

	void VulkanDrawable::SetRenderDevice(VulkanRenderer* renderer) {
		pRenderer = renderer;
		vertexBuffer = { pRenderer->device_, vkDestroyBuffer };
		vertexBufferMemory = { pRenderer->device_, vkFreeMemory };
		indexBuffer = { pRenderer->device_, vkDestroyBuffer };
		indexBufferMemory = { pRenderer->device_, vkFreeMemory };
		uniformStagingBuffer = { pRenderer->device_, vkDestroyBuffer };
		uniformStagingBufferMemory = { pRenderer->device_, vkFreeMemory };
		uniformBuffer = { pRenderer->device_, vkDestroyBuffer };
		uniformBufferMemory = { pRenderer->device_, vkFreeMemory };
		descriptorPool = { pRenderer->device_, vkDestroyDescriptorPool };
		descriptorSetLayout = { pRenderer->device_, vkDestroyDescriptorSetLayout };
		pPipeline = { pRenderer->device_, vkDestroyPipeline };
	}

	void VulkanDrawable::Generate(const std::vector<VertexPTN>& vertices, const std::vector<uint32_t>& indices, const Material& material) {
		if (!pRenderer) {
			throw std::runtime_error("Vulkan Drawable not intialized!");
		}

		mNumIndices = static_cast<uint32_t>(indices.size());
		CreateVertexBuffer(vertices);
		CreateIndexBuffer(indices);
		CreateDescriptorSetLayout();
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vertices[0].GetAttributeDescriptions();

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{ vertices[0].GetBindingDescription() };
		std::vector<VkDescriptorSetLayout> layouts = { descriptorSetLayout };

		pRenderer->pipeline.CreatePipeline(pPipeline, attributeDescriptions, bindingDescriptions, layouts, material.shader);
		CreateUniformBuffer();
		CreateDescriptorPool();
		CreateDescriptorSet(material);
	}

	void VulkanDrawable::RecordDrawCommand(const VkCommandBuffer& commandBuffer) const {
		if (mNumIndices == 0 || !pPipeline) { return; }

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pRenderer->pipeline.pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, mNumIndices, 1, 0, 0, 0);
	}

	void VulkanDrawable::CreateVertexBuffer(const std::vector<VertexPTN>& vertices) {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkCom<VkBuffer> stagingBuffer{ pRenderer->device_, vkDestroyBuffer };
		VkCom<VkDeviceMemory> stagingBufferMemory{ pRenderer->device_, vkFreeMemory };
		pRenderer->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(pRenderer->device_, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(pRenderer->device_, stagingBufferMemory);

		pRenderer->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		pRenderer->CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);
	}

	void VulkanDrawable::CreateIndexBuffer(const std::vector<uint32_t>& indices) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkCom<VkBuffer> stagingBuffer{ pRenderer->device_, vkDestroyBuffer };
		VkCom<VkDeviceMemory> stagingBufferMemory{ pRenderer->device_, vkFreeMemory };
		pRenderer->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(pRenderer->device_, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(pRenderer->device_, stagingBufferMemory);

		pRenderer->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		pRenderer->CopyBuffer(stagingBuffer, indexBuffer, bufferSize);
	}

	void VulkanDrawable::CreateDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding;
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding;
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(pRenderer->device_, &layoutInfo, nullptr, descriptorSetLayout.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanDrawable::CreateUniformBuffer() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		pRenderer->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer, uniformStagingBufferMemory);
		pRenderer->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer, uniformBufferMemory);
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

		if (vkCreateDescriptorPool(pRenderer->device_, &poolInfo, nullptr, descriptorPool.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanDrawable::CreateDescriptorSet(const Material& material) {
		VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		if (vkAllocateDescriptorSets(pRenderer->device_, &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		VulkanTexture* diffuseTexture = pRenderer->GetTexture(material.diffuseTexture);
		if (diffuseTexture) {
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSet;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &diffuseTexture->imageInfo;
		}

		vkUpdateDescriptorSets(pRenderer->device_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
