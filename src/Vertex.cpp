#include "Vertex.h"

VkVertexInputBindingDescription VertexPTC::GetBindingDescription() const {
	VkVertexInputBindingDescription bindingDescription = {
		0,
		sizeof(VertexPTC),
		VK_VERTEX_INPUT_RATE_VERTEX
	};

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VertexPTC::GetAttributeDescriptions() const {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = std::vector<VkVertexInputAttributeDescription>(3);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(VertexPTC, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(VertexPTC, texCoord);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(VertexPTC, color);

	return attributeDescriptions;
}

bool VertexPTC::operator==(const VertexPTC& other) const {
	return position == other.position && color == other.color && texCoord == other.texCoord;
}

VkVertexInputBindingDescription VertexPTN::GetBindingDescription() const {
	VkVertexInputBindingDescription bindingDescription = {
		0,
		sizeof(VertexPTN),
		VK_VERTEX_INPUT_RATE_VERTEX
	};

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VertexPTN::GetAttributeDescriptions() const {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = std::vector<VkVertexInputAttributeDescription>(3);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(VertexPTN, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(VertexPTN, texCoord);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(VertexPTN, normal);

	return attributeDescriptions;
}

bool VertexPTN::operator==(const VertexPTN& other) const {
	return position == other.position && normal == other.normal && texCoord == other.texCoord;
}

VkVertexInputBindingDescription VertexPTNTC::GetBindingDescription() const {
	VkVertexInputBindingDescription bindingDescription = {
		0,
		sizeof(VertexPTNTC),
		VK_VERTEX_INPUT_RATE_VERTEX
	};

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VertexPTNTC::GetAttributeDescriptions() const {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = std::vector<VkVertexInputAttributeDescription>(5);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(VertexPTNTC, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(VertexPTNTC, texCoord);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(VertexPTNTC, normal);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(VertexPTNTC, tangent);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(VertexPTNTC, color);

	return attributeDescriptions;
}

bool VertexPTNTC::operator==(const VertexPTNTC& other) const {
	return position == other.position && normal == other.normal && texCoord == other.texCoord && tangent == other.tangent;
}

