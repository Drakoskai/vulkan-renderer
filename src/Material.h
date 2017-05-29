#pragma once

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
			/*^ (alphaTexture == EmptyTextureId ? 0 : 1) >> 1
			^ (ambientTexture == EmptyTextureId ? 0 : 1) >> 1*/
			^ (diffuseTexture == EmptyTextureId ? 0 : 1) >> 1
			/*^ (specularTexture == EmptyTextureId ? 0 : 1) >> 1
			^ (specularHighlightTexture == EmptyTextureId ? 0 : 1) >> 1
			^ (bumpTexture == EmptyTextureId ? 0 : 1) >> 1
			^ (displacementTexture == EmptyTextureId ? 0 : 1)*/) >> 1;

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

struct MaterialGroup {
	std::vector<Vertex> vertices = {};
	std::vector<uint32_t> indices = {};
	Material material;

};
