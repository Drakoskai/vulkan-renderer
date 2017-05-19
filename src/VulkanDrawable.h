#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VkCom.h"
#include "Vertex.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanVertexBuffer.h"
#include "Material.h"

namespace Vulkan {
	class VulkanRenderer;

	class VulkanDrawable {
	public:
		VulkanDrawable();
		VulkanDrawable(VulkanRenderer* renderer);
		~VulkanDrawable();
		void SetRenderDevice(VulkanRenderer* renderer);
		void Generate(const std::vector<VertexPTN>& vertices, const std::vector<uint32_t>& indices, const Material& material);
		void RecordDrawCommand(const VkCommandBuffer& commandBuffer) const;
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSet();	
		const VkCom<VkBuffer>& GetUniformBuffer() const { return uniformBuffer_; }
		const VkCom<VkBuffer>& GetUniformStagingBuffer() const { return uniformStagingBuffer_; }
		const VkCom<VkDeviceMemory>& GetUniformBufferMemory() const { return uniformBufferMemory_; }
		const VkCom<VkDeviceMemory>& GetUniformStagingMemory() const { return uniformStagingBufferMemory_; }

	private:
		VulkanVertexBuffer vertexBuffer_;
		VkCom<VkBuffer> uniformBuffer_;
		VkCom<VkBuffer> uniformStagingBuffer_;
		VkCom<VkDeviceMemory> uniformStagingBufferMemory_;
		VkCom<VkDeviceMemory> uniformBufferMemory_;
		VkCom<VkDescriptorPool> descriptorPool_;
		VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
		Material material_;
		uint32_t numIndices_;
		VulkanPipeline* pPipeline_;
		VulkanRenderer* pRenderer_;
	};
}
