#ifndef VERTEX_H__
#define VERTEX_H__

#include <array>
#include <vector>
#include "KaiMath.h"
#include "VkCom.h"

struct Vertex {
	Vec3 position;
	Vec2 texCoord;
	Vec3 normal;

	static VkVertexInputBindingDescription GetBindingDescription();
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
