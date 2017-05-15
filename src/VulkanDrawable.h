#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VkCom.h"
#include "Vertex.h"
#include "VulkanPipeline.h"

namespace Vulkan {
	class VulkanRenderer;

	class VulkanDrawable {
	public:
		VulkanDrawable();
		VulkanDrawable(VulkanRenderer* renderer);
		~VulkanDrawable();
		void SetRenderDevice(VulkanRenderer* renderer);
		void Generate(const std::vector<VertexPTC>& vertices, const std::vector<uint32_t>& indices);
		void RecordDrawCommand(const VkCommandBuffer& commandBuffer) const;
		void CreateVertexBuffer(const std::vector<VertexPTC>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
		void CreateDescriptorSetLayout();
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSet();

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
		VkDescriptorSet descriptorSet = nullptr;
		VkCom<VkPipeline> pPipeline;

		VulkanRenderer* pRenderer;
		
	private:
		uint32_t mNumIndices;
	};
}
