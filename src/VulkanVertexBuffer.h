#ifndef VULKAN_VERTEX_BUFFER_H_
#define VULKAN_VERTEX_BUFFER_H_

#include <vector>
#include "VkCom.h"
#include "Vertex.h"
#include "VulkanPipeline.h"
#include "SubMesh.h"

namespace Vulkan {

	struct VertexBufferSection {
		VkBuffer buffer = VK_NULL_HANDLE;
		uint32_t offset = 0;
		uint32_t idxoffset = 0;
		uint32_t size = 0;
		uint64_t materialId = 0L;
		VulkanPipeline* pipeline;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	class VulkanVertexBuffer {
	public:
		VulkanVertexBuffer();
		~VulkanVertexBuffer();
		void AddGeometry(const SubMesh& group, VulkanPipeline* pipeline);
		void Generate();
		const VkCom<VkBuffer>& GetVertexBuffer() const { return vertexBuffer_; }
		const VkCom<VkBuffer>& GetIndexBuffer() const { return indexBuffer_; }
		const std::vector<VkVertexInputAttributeDescription>& GetattributeDescriptions() const { return attributeDescriptions_; }
		const std::vector<VkVertexInputBindingDescription>& GetBindingDescriptions() const { return bindingDescriptions_; }	
		std::vector<VertexBufferSection> vertexBufferSections_;	
	private:
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
		VkCom<VkBuffer> vertexBuffer_{ VK_NULL_HANDLE };
		VkCom<VkDeviceMemory> vertexBufferMemory_{ VK_NULL_HANDLE };
		VkCom<VkBuffer> indexBuffer_{ VK_NULL_HANDLE };
		VkCom<VkDeviceMemory> indexBufferMemory_{ VK_NULL_HANDLE };
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions_;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions_;
		std::vector<std::pair<SubMesh, VulkanPipeline*>> geometries_;
	};
}
#endif

