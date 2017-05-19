#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VkCom.h"
#include "Vertex.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanVertexBuffer.h"

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

		VulkanVertexBuffer vertexBuffer_;
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
		uint32_t numIndices_;
	};
}
