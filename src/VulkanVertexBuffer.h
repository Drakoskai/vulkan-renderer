#ifndef VULKAN_VERTEX_BUFFER_H_
#define VULKAN_VERTEX_BUFFER_H_

#include <vector>
#include "VkCom.h"
#include "Vertex.h"

namespace Vulkan {
	class VulkanRenderer;
	class VulkanVertexBuffer {
	public:
		VulkanVertexBuffer();
		~VulkanVertexBuffer();
		void SetRenderer(VulkanRenderer* renderer);
		void Generate(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);		
		const VkCom<VkBuffer>& GetVertexBuffer() const { return vertexBuffer_; }
		const VkCom<VkBuffer>& GetIndexBuffer() const { return indexBuffer_; }
		const std::vector<VkVertexInputAttributeDescription>& GetattributeDescriptions() const { return attributeDescriptions_; }
		const std::vector<VkVertexInputBindingDescription>& GetBindingDescriptions() const { return bindingDescriptions_; }
	private:
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
		VkCom<VkBuffer> vertexBuffer_{ VK_NULL_HANDLE };
		VkCom<VkDeviceMemory> vertexBufferMemory_{ VK_NULL_HANDLE };
		VkCom<VkBuffer> indexBuffer_{ VK_NULL_HANDLE };
		VkCom<VkDeviceMemory> indexBufferMemory_{ VK_NULL_HANDLE };
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions_;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions_;
		VulkanRenderer* pRenderer_;
	};
}
#endif

