#include "VulkanDrawable.h"

#include "VulkanUtil.h"
#include "VulkanRenderSystem.h"
#include "VulkanTexture.h"

namespace Vulkan {

	VulkanDrawable::VulkanDrawable() : numIndices_(0) {}

	VulkanDrawable::~VulkanDrawable() {}

	void VulkanDrawable::Generate(std::vector<SubMesh>& subMeshes) {
		uniformStagingBuffer_ = { VulkanRenderSystem::Device, vkDestroyBuffer };
		uniformStagingBufferMemory_ = { VulkanRenderSystem::Device, vkFreeMemory };
		uniformBuffer_ = { VulkanRenderSystem::Device, vkDestroyBuffer };
		uniformBufferMemory_ = { VulkanRenderSystem::Device, vkFreeMemory };
		descriptorPool_ = { VulkanRenderSystem::Device, vkDestroyDescriptorPool };

		for (auto submesh : subMeshes) {
			size_t materialHash = submesh.material.HashCode();
			if (Material::materials.find(materialHash) == end(Material::materials)) {
				Material::materials[materialHash] = submesh.material;
			}
		}

		CreateUniformBuffer();

		VulkanPipeline* pipeline = VulkanPipeline::GetPipeline(0L);

		for (auto submesh : subMeshes) {
			vertexBuffer_.AddGeometry(submesh, pipeline);
			std::vector<VkVertexInputBindingDescription> bindingDescriptions = { submesh.vertices[0].GetBindingDescription() };
			std::vector<VkVertexInputAttributeDescription> attribDescriptions = submesh.vertices[0].GetAttributeDescriptions();

			if (!pipeline->IsGenerated()) {
				pipeline->CreatePipeline(bindingDescriptions, attribDescriptions, submesh.material.shader);
			}
		}
		uint32_t descriptorsCount = static_cast<uint32_t>(Material::materials.size());
		CreateDescriptorPool(descriptorsCount);
		for (auto material : Material::materials) {
			size_t materialKey = material.first;
			if (materialDescriptors_.find(materialKey) == end(materialDescriptors_)) {
				materialDescriptors_[materialKey] = CreateDescriptorSet(pipeline);
				AllocateDescriptorSet(materialDescriptors_[materialKey], materialKey);
			}
		}

		vertexBuffer_.Generate();
	}

	void VulkanDrawable::RecordDrawCommand(const VkCommandBuffer& commandBuffer) {
		for (auto section : vertexBuffer_.vertexBufferSections_) {
			VulkanPipeline* pipeline = section.pipeline;
			size_t materialKey = section.materialId;
			Material material = Material::materials[materialKey];
			UniformMaterialObject pushConstants(material);
			vkCmdPushConstants(commandBuffer, pipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), &pushConstants);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipelineLayout(), 0, 1, &materialDescriptors_[materialKey], 0, nullptr);
			VkBuffer vertexBuffers[] = { vertexBuffer_.GetVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, vertexBuffer_.GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, section.size, 1, 0, section.offset, 0);
		}
	}

	void VulkanDrawable::CreateUniformBuffer() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		VulkanRenderSystem::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer_, uniformStagingBufferMemory_);
		VulkanRenderSystem::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer_, uniformBufferMemory_);
	}

	void VulkanDrawable::CreateDescriptorPool(uint32_t decriptorCount) {
		std::array<VkDescriptorPoolSize, 3> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = decriptorCount;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = decriptorCount;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = decriptorCount;

		VkDescriptorPoolCreateInfo poolInfo;
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pNext = nullptr;
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = decriptorCount + 1;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		if (vkCreateDescriptorPool(VulkanRenderSystem::Device, &poolInfo, nullptr, descriptorPool_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	VkDescriptorSet VulkanDrawable::CreateDescriptorSet(VulkanPipeline* pipeline) const {
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

		std::array<VkDescriptorSetLayout, 1> layouts = { pipeline->GetDescriptorSetLayout() };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool_;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(VulkanRenderSystem::Device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}
		return descriptorSet;
	}

	VkDescriptorSet VulkanDrawable::AllocateDescriptorSet(VkDescriptorSet descriptorSet, size_t materialHash) const {
		VkDescriptorBufferInfo bufferInfo;

		Material material = Material::materials[materialHash];

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

		VulkanTexture* diffuseTexture = VulkanTexture::GetTexture(material.diffuseTexture);

		VkWriteDescriptorSet textureDescriptorDescriptor;
		textureDescriptorDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureDescriptorDescriptor.pNext = nullptr;
		textureDescriptorDescriptor.dstSet = descriptorSet;
		textureDescriptorDescriptor.dstBinding = 1;
		textureDescriptorDescriptor.dstArrayElement = 0;
		textureDescriptorDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureDescriptorDescriptor.descriptorCount = 1;
		textureDescriptorDescriptor.pImageInfo = &diffuseTexture->GetImageInfo();
		descriptorWrites.push_back(textureDescriptorDescriptor);

		VulkanTexture* normalTexture = VulkanTexture::GetTexture(material.bumpTexture);
		VkWriteDescriptorSet textureDescriptor;
		textureDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureDescriptor.pNext = nullptr;
		textureDescriptor.dstSet = descriptorSet;
		textureDescriptor.dstBinding = 2;
		textureDescriptor.dstArrayElement = 0;
		textureDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureDescriptor.descriptorCount = 1;
		textureDescriptor.pImageInfo = &normalTexture->GetImageInfo();
		descriptorWrites.push_back(textureDescriptor);

		vkUpdateDescriptorSets(VulkanRenderSystem::Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		return descriptorSet;
	}
}
