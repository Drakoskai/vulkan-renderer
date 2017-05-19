#ifndef VULKAN_VERTEX_BUFFER_H_
#define VULKAN_VERTEX_BUFFER_H_
#include "VkCom.h"
#include <vector>
#include "Vertex.h"

namespace Vulkan {
	class VulkanRenderer;
	struct VulkanVertexBuffer {
		VkCom<VkBuffer> vertexBuffer_;
		VkCom<VkDeviceMemory> vertexBufferMemory_;
		VkCom<VkBuffer> indexBuffer_;
		VkCom<VkDeviceMemory> indexBufferMemory_;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions_;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions_;
		VulkanRenderer* pRenderer_;

		VulkanVertexBuffer();
		void SetRenderer(VulkanRenderer* renderer);
		void Generate(const std::vector<VertexPTN>& vertices, const std::vector<uint32_t>& indices);
		void CreateVertexBuffer(const std::vector<VertexPTN>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
	};
}
#endif

