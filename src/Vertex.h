#pragma once

#include "KaiMath.h"
#include <array>
#include <vector>
#include "VkCom.h"

enum class VertexFormat {
	PTC, PTN, PTNTC
};

struct Vertex {
	virtual ~Vertex() { }
	virtual VkVertexInputBindingDescription GetBindingDescription() = 0;
	virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() = 0;
};

struct VertexPTC {
	Vec3 position;
	Vec2 texCoord;
	Vec4 color;

	static VkVertexInputBindingDescription GetBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();

	bool operator==(const VertexPTC& other) const;
};

namespace std {
	template<> struct hash<VertexPTC> {
		size_t operator()(VertexPTC const& vertex) const {
			return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.texCoord) << 1 ^ hash<glm::vec4>()(vertex.color) << 1) >> 1;
		}
	};
}

struct VertexPTN {
	Vec3 position;
	Vec2 texCoord;
	Vec3 normal;

	static VkVertexInputBindingDescription GetBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();

	bool operator==(const VertexPTN& other) const;
};

namespace std {
	template<> struct hash<VertexPTN> {
		size_t operator()(VertexPTN const& vertex) const {
			return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.texCoord) << 1 ^ hash<glm::vec3>()(vertex.normal) << 1) >> 1;
		}
	};
}

struct VertexPTNTC {
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 tangent;
	glm::vec4 color;

	static VkVertexInputBindingDescription GetBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions();
	bool operator==(const VertexPTNTC& other) const;
};

namespace std {
	template<> struct hash<VertexPTNTC> {
		size_t operator()(VertexPTNTC const& vertex) const {
			return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.texCoord) << 1 ^ hash<glm::vec3>()(vertex.normal) << 1) >> 1 ^ hash<glm::vec4>()(vertex.tangent) << 1 ^ hash<glm::vec4>()(vertex.color) << 1;
		}
	};
}
