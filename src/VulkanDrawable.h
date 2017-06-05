#ifndef VULKAN_DRAWABLE_H__
#define VULKAN_DRAWABLE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include "VkCom.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanVertexBuffer.h"
#include "Material.h"
#include "SubMesh.h"

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
		VkDescriptorSet CreateDescriptorSet(VulkanPipeline* pipeline) const;
		VkDescriptorSet AllocateDescriptorSet(VkDescriptorSet descriptorSet, size_t materialHash);
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
		uint32_t nextDescriptor = 0;
		VulkanVertexBuffer vertexBuffer_;
		std::unordered_map<size_t, Material> materials_;
		std::unordered_map<size_t, VkDescriptorSet> materialDescriptors_;
		VulkanRenderer* pRenderer_;
	};
}
#endif
