#include "VulkanDrawable.h"
#include "VulkanUtil.h"
#include "VulkanRenderer.h"
#include <set>
#include <unordered_set>

namespace Vulkan {

	VulkanDrawable::VulkanDrawable() : numIndices_(0), pRenderer_(nullptr) {}

	VulkanDrawable::VulkanDrawable(VulkanRenderer* renderer) : numIndices_(0), pRenderer_(renderer) {
		uniformStagingBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformStagingBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		uniformBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		descriptorPool_ = { pRenderer_->device_, vkDestroyDescriptorPool };
	}

	VulkanDrawable::~VulkanDrawable() {}

	void VulkanDrawable::SetRenderDevice(VulkanRenderer* renderer) {
		pRenderer_ = renderer;
		uniformStagingBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformStagingBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		uniformBuffer_ = { pRenderer_->device_, vkDestroyBuffer };
		uniformBufferMemory_ = { pRenderer_->device_, vkFreeMemory };
		descriptorPool_ = { pRenderer_->device_, vkDestroyDescriptorPool };
	}

	void VulkanDrawable::Generate(std::vector<SubMesh>& subMeshes) {
		if (!pRenderer_) {
			throw std::runtime_error("Vulkan Drawable not intialized!");
		}
		uint32_t size = static_cast<uint32_t>(subMeshes.size());
		vertexBuffer_.SetRenderer(pRenderer_);

		for (auto submesh : subMeshes) {
			for (auto group : submesh.groups) {
				size_t materialHash = group.material.HashCode();
				if (materials_.find(materialHash) == end(materials_)) {
					materials_[materialHash] = group.material;
				}
			}
		}

		CreateUniformBuffer();

		uint32_t descriptorsCount = static_cast<uint32_t>(materials_.size());
		CreateDescriptorPool(descriptorsCount);

		for (auto submesh : subMeshes) {
			for (uint32_t i = 0; i < submesh.groups.size(); i++) {
				auto group = submesh.groups[i];
			
				VulkanPipeline* pipeline = pRenderer_->GetPipeline(0L);
				vertexBuffer_.AddGeometry(group, pipeline);
				size_t materialHash = group.material.HashCode();
				std::vector<VkVertexInputBindingDescription> bindingDescriptions = { group.vertices[0].GetBindingDescription() };
				std::vector<VkVertexInputAttributeDescription> attribDescriptions = group.vertices[0].GetAttributeDescriptions();

				if (!pipeline->IsGenerated()) {
					pipeline->CreatePipeline(bindingDescriptions, attribDescriptions, group.material.shader);
				}

				if (descriptorSets_.find(materialHash) == end(descriptorSets_)) {
					CreateDescriptorSet(pipeline, materialHash);
				}
			}
		}

		vertexBuffer_.Generate();
	}

	void VulkanDrawable::RecordDrawCommand(const VkCommandBuffer& commandBuffer) {	
		for (auto section : vertexBuffer_.vertexBufferSections_) {
			VulkanPipeline* pipeline = section.pipeline;
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
			VkBuffer vertexBuffers[] = { vertexBuffer_.GetVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, vertexBuffer_.GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			VkDescriptorSet descriptorSet = descriptorSets_[section.materialId];
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
			vkCmdDrawIndexed(commandBuffer, section.size, 1, section.idxoffset, section.offset, 0);
		}
	}

	void VulkanDrawable::CreateUniformBuffer() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer_, uniformStagingBufferMemory_);
		pRenderer_->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer_, uniformBufferMemory_);
	}

	void VulkanDrawable::CreateDescriptorPool(uint32_t decriptorCount) {
		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = decriptorCount;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = decriptorCount;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = decriptorCount;

		if (vkCreateDescriptorPool(pRenderer_->device_, &poolInfo, nullptr, descriptorPool_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanDrawable::CreateDescriptorSet(VulkanPipeline* pipeline, size_t materialHash) {
		VkDescriptorSetLayout layouts[] = { pipeline->GetDescriptorSetLayout() };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool_;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		if (vkAllocateDescriptorSets(pRenderer_->device_, &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}

		descriptorSets_[materialHash] = descriptorSet;

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer_;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		std::vector<VkWriteDescriptorSet> descriptorWrites;
		VkWriteDescriptorSet uboDescriptor;

		uboDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uboDescriptor.pNext = nullptr;
		uboDescriptor.dstSet = descriptorSet;
		uboDescriptor.dstBinding = 0;
		uboDescriptor.dstArrayElement = 0;
		uboDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboDescriptor.descriptorCount = 1;
		uboDescriptor.pBufferInfo = &bufferInfo;

		descriptorWrites.push_back(uboDescriptor);

		VulkanTexture* diffuseTexture = pRenderer_->GetTexture(materials_[materialHash].diffuseTexture);
		if (diffuseTexture) {
			VkWriteDescriptorSet texturDescriptor;
			texturDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			texturDescriptor.pNext = nullptr;
			texturDescriptor.dstSet = descriptorSet;
			texturDescriptor.dstBinding = 1;
			texturDescriptor.dstArrayElement = 0;
			texturDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texturDescriptor.descriptorCount = 1;
			texturDescriptor.pImageInfo = &diffuseTexture->imageInfo_;
			descriptorWrites.push_back(texturDescriptor);
		}

		vkUpdateDescriptorSets(pRenderer_->device_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
