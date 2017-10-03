#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <unordered_map>
#include "KaiMath.h"
#include "GfxTypes.h"

struct Material {
	static std::unordered_map<uint64_t, Material> materials;
	std::string name;
	Vec3 ambient;
	Vec3 diffuse;
	Vec3 emissive;
	Vec3 specular;
	float reflection;
	float opacity;
	float shininess;
	TextureId alphaTexture;
	TextureId ambientTexture;
	TextureId diffuseTexture;
	TextureId specularTexture;
	TextureId specularHighlightTexture;
	TextureId bumpTexture;
	TextureId displacementTexture;
	ShaderId shader;

	uint64_t HashCode() const {
		const uint64_t nameHash = (std::hash<std::string>()(name) ^
			std::hash<glm::vec3>()(ambient) << 1
			^ std::hash<glm::vec3>()(diffuse) << 1
			^ std::hash<glm::vec3>()(emissive) << 1
			^ std::hash<glm::vec3>()(specular) << 1
			^ std::hash<float>()(reflection) << 1
			^ std::hash<float>()(opacity) << 1
			^ std::hash<float>()(shininess) << 1
			^ diffuseTexture.HashCode()) >> 1;

		return nameHash;
	}

	bool operator==(const Material& other) const {
		return HashCode() == other.HashCode();
	}
};

namespace std {
	template<> struct hash<Material> {
		size_t operator()(Material const& material) const {
			return material.HashCode();
		}
	};
}

struct UniformMaterialObject {
	Vec3 ambient;
	Vec3 diffuse;
	Vec3 emissive;
	Vec3 specular;
	float reflection;
	float opacity;
	float shininess;
	UniformMaterialObject() : reflection(0), opacity(0), shininess(0) {}

	UniformMaterialObject(Material material) :
		ambient(material.ambient),
		diffuse(material.diffuse),
		emissive(material.emissive),
		specular(material.specular),
		reflection(material.reflection),
		opacity(material.opacity),
		shininess(material.shininess) {}
};

#endif
