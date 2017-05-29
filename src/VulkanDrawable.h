#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VkCom.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanVertexBuffer.h"
#include "Material.h"
#include "SubMesh.h"
#include <unordered_map>

namespace Vulkan {
	class VulkanRenderer;

	class VulkanDrawable {
	public:
		VulkanDrawable();
		VulkanDrawable(VulkanRenderer* renderer);
		~VulkanDrawable();
		void SetRenderDevice(VulkanRenderer* renderer);
		void VulkanDrawable::Generate(std::vector<SubMesh>& subMeshes);
		void RecordDrawCommand(const VkCommandBuffer& commandBuffer);
		void CreateUniformBuffer();
		void CreateDescriptorPool(uint32_t decriptorCount);
		void CreateDescriptorSet(VulkanPipeline* pipeline, size_t materialHash);
		const VkCom<VkBuffer>& GetUniformBuffer() const { return uniformBuffer_; }
		const VkCom<VkBuffer>& GetUniformStagingBuffer() const { return uniformStagingBuffer_; }
		const VkCom<VkDeviceMemory>& GetUniformBufferMemory() const { return uniformBufferMemory_; }
		const VkCom<VkDeviceMemory>& GetUniformStagingMemory() const { return uniformStagingBufferMemory_; }

	private:
		VkCom<VkBuffer> uniformBuffer_;
		VkCom<VkBuffer> uniformStagingBuffer_;
		VkCom<VkDeviceMemory> uniformStagingBufferMemory_;
		VkCom<VkDeviceMemory> uniformBufferMemory_;
		VkCom<VkDescriptorPool> descriptorPool_;

		uint32_t numIndices_;
		VulkanVertexBuffer vertexBuffer_;
		std::unordered_map<size_t, VkDescriptorSet> descriptorSets_;
		std::unordered_map<size_t, Material> materials_;
		VulkanRenderer* pRenderer_;
	};
}
