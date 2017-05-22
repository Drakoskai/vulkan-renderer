#pragma once

#include "Vertex.h"
#include <string>
#include "GraphicsStructs.h"

struct Material {
	std::string name;
	Vec3 ambient;
	Vec3 diffuse;
	Vec3 emissive;
	Vec3 specular;
	float shininess;
	float alpha;
	TextureId diffuseTexture;
	ShaderId shader;
};
