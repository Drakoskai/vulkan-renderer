#pragma once

#include "KaiMath.h"
#include <array>
#include <vector>
#include "VkCom.h"

enum class VertexFormat {
	PTC, PTN, PTNTC
};

struct Vertex {
	Vec3 position;
	Vertex() {}
	virtual ~Vertex() = default;
	virtual VkVertexInputBindingDescription GetBindingDescription() const {
		VkVertexInputBindingDescription empty{};
		return empty;
	}

	virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const = 0 {
		std::vector<VkVertexInputAttributeDescription> empty{};
		return empty;
	}
};

struct VertexPTC : Vertex {
	Vec2 texCoord;
	Vec4 color;

	VkVertexInputBindingDescription GetBindingDescription() const override;
	std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()const override;

	bool operator==(const VertexPTC& other) const;
};

namespace std {
	template<> struct hash<VertexPTC> {
		size_t operator()(VertexPTC const& vertex) const {
			return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.texCoord) << 1 ^ hash<glm::vec4>()(vertex.color) << 1) >> 1;
		}
	};
}

struct VertexPTN : Vertex {
	Vec2 texCoord;
	Vec3 normal;

	VkVertexInputBindingDescription GetBindingDescription() const override;
	std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const override;

	bool operator==(const VertexPTN& other) const;
};

namespace std {
	template<> struct hash<VertexPTN> {
		size_t operator()(VertexPTN const& vertex) const {
			return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.texCoord) << 1 ^ hash<glm::vec3>()(vertex.normal) << 1) >> 1;
		}
	};
}

struct VertexPTNTC : Vertex {
	Vec2 texCoord;
	Vec3 normal;
	Vec4 tangent;
	Vec4 color;

	VkVertexInputBindingDescription GetBindingDescription() const override;
	std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const override;
	bool operator==(const VertexPTNTC& other) const;
};

namespace std {
	template<> struct hash<VertexPTNTC> {
		size_t operator()(VertexPTNTC const& vertex) const {
			return (hash<glm::vec3>()(vertex.position) ^ hash<glm::vec2>()(vertex.texCoord) << 1 ^ hash<glm::vec3>()(vertex.normal) << 1) >> 1 ^ hash<glm::vec4>()(vertex.tangent) << 1 ^ hash<glm::vec4>()(vertex.color) << 1;
		}
	};
}
