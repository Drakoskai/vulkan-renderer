#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VkCom.h"
#include "Vertex.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

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
		void CreateVertexBuffer(const std::vector<VertexPTN>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
		void CreateDescriptorSetLayout();
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSet(const Material& material);

		VkCom<VkBuffer> vertexBuffer;
		VkCom<VkDeviceMemory> vertexBufferMemory;
		VkCom<VkBuffer> indexBuffer;
		VkCom<VkDeviceMemory> indexBufferMemory;
		VkCom<VkBuffer> uniformStagingBuffer;
		VkCom<VkDeviceMemory> uniformStagingBufferMemory;

		VkCom<VkBuffer> uniformBuffer;
		VkCom<VkDeviceMemory> uniformBufferMemory;

		VkCom<VkDescriptorPool> descriptorPool;		
		VkCom<VkDescriptorSetLayout> descriptorSetLayout;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		//VulkanPipeline* pPipeline;
		VkCom<VkPipeline> pPipeline;
		VulkanRenderer* pRenderer;
		
	private:
		uint32_t mNumIndices;
	};
}
