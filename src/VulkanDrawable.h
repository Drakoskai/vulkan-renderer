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
		void CreateUniformBuffer();
		void CreateDescriptorPool();
		void CreateDescriptorSet();

		VkCom<VkBuffer> vertexBuffer_;
		VkCom<VkDeviceMemory> vertexBufferMemory_;
		VkCom<VkBuffer> indexBuffer_;
		VkCom<VkDeviceMemory> indexBufferMemory_;
		VkCom<VkBuffer> uniformStagingBuffer_;
		VkCom<VkDeviceMemory> uniformStagingBufferMemory_;

		VkCom<VkBuffer> uniformBuffer_;
		VkCom<VkDeviceMemory> uniformBufferMemory_;

		VkCom<VkDescriptorPool> descriptorPool_;		
		

		VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
		VulkanPipeline* pPipeline_;
		VulkanRenderer* pRenderer_;
		Material material_;
		
	private:
		uint32_t mNumIndices_;
	};
}
