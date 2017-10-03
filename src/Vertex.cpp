#include "Vertex.h"
#include <stddef.h>

std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescription() {
	const VkVertexInputBindingDescription bindingDescription = {
		0,
		sizeof(Vertex),
		VK_VERTEX_INPUT_RATE_VERTEX
	};

	return std::vector<VkVertexInputBindingDescription> {bindingDescription};
}

std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	
	VkVertexInputAttributeDescription position{};
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32G32B32_SFLOAT;
	position.offset = offsetof(Vertex, position);
	attributeDescriptions.push_back(position);
	
	VkVertexInputAttributeDescription texCoord0{};
	texCoord0.binding = 0;
	texCoord0.location = 1;
	texCoord0.format = VK_FORMAT_R32G32_SFLOAT;
	texCoord0.offset = offsetof(Vertex, texCoord);
	attributeDescriptions.push_back(texCoord0);

	VkVertexInputAttributeDescription normal{};
	normal.binding = 0;
	normal.location = 2;
	normal.format = VK_FORMAT_R32G32B32_SFLOAT;
	normal.offset = offsetof(Vertex, normal);
	attributeDescriptions.push_back(normal);

	return attributeDescriptions;
}

bool Vertex::operator==(const Vertex& other) const {
	return position == other.position && normal == other.normal && texCoord == other.texCoord;
}
