#ifndef MATERIAL_H__
#define MATERIAL_H__

#include "Vertex.h"
#include "GfxTypes.h"

struct Material {
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

	uint32_t GetNumberOfTextures() const {
		TextureId empty;
		uint32_t numTextures = 0;
		if (alphaTexture != empty) {
			numTextures++;
		}
		if (ambientTexture != empty) {
			numTextures++;
		}
		if (diffuseTexture != empty) {
			numTextures++;
		}
		if (specularTexture != empty) {
			numTextures++;
		}
		if (specularHighlightTexture != empty) {
			numTextures++;
		}
		if (bumpTexture != empty) {
			numTextures++;
		}
		if (displacementTexture != empty) {
			numTextures++;
		}
		return numTextures;
	}

	size_t HashCode() const {
		size_t nameHash = (std::hash<std::string>()(name) ^
			std::hash<glm::vec3>()(ambient) << 1
			^ std::hash<glm::vec3>()(diffuse) << 1
			^ std::hash<glm::vec3>()(emissive) << 1
			^ std::hash<glm::vec3>()(specular) << 1
			^ std::hash<float>()(reflection) << 1
			^ std::hash<float>()(opacity) << 1
			^ std::hash<float>()(shininess) << 1
			^ alphaTexture.HashCode() << 1
			^ ambientTexture.HashCode() << 1
			^ diffuseTexture.HashCode() << 1
			^ specularTexture.HashCode() << 1
			^ specularHighlightTexture.HashCode() << 1
			^ bumpTexture.HashCode() << 1
			^ displacementTexture.HashCode()) >> 1;

		return nameHash;
	}

	size_t GetDescriptorHash() const {
		uint32_t numTextures = 0;

		if (alphaTexture != EmptyTextureId) {
			numTextures++;
		}
		if (diffuseTexture != EmptyTextureId) {
			numTextures++;
		}
		if (specularTexture != EmptyTextureId) {
			numTextures++;
		}
		if (specularHighlightTexture != EmptyTextureId) {
			numTextures++;
		}
		if (bumpTexture != EmptyTextureId) {
			numTextures++;
		}
		if (displacementTexture != EmptyTextureId) {
			numTextures++;
		}

		size_t hash = (std::hash<std::string>()(name) ^
			std::hash<uint32_t>()(ambient != Vec3(0.0f) ? 1 : 0) << 1
			^ std::hash<uint32_t>()(diffuse != Vec3(0.0f) ? 1 : 0) << 1
			^ std::hash<uint32_t>()(emissive != Vec3(0.0f) ? 1 : 0) << 1
			^ std::hash<uint32_t>()(specular != Vec3(0.0f) ? 1 : 0) << 1
			^ std::hash<uint32_t>()(reflection != 0.0f ? 1 : 0) << 1
			^ std::hash<uint32_t>()(opacity != 0.0f ? 1 : 0) << 1
			^ std::hash<uint32_t>()(shininess != 0.0f ? 1 : 0) << 1
			^ std::hash<uint32_t>()(numTextures) << 1) >> 1;
		return hash;
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
#endif
