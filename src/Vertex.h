#ifndef VERTEX_H_
#define VERTEX_H_

#include <vulkan/vulkan.h>
#include <array>
#include <vector>
#include "KaiMath.h"

enum VertexType {
	DrawVertex
};

struct Vertex {
	Vec3 position;
	Vec2 texCoord;
	Vec3 normal;

	static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	bool operator==(const Vertex& other) const;
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.texCoord) << 1 ^ hash<glm::vec3>()(vertex.normal) << 1) >> 1;
		}
	};
}
#endif
