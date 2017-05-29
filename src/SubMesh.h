#pragma once

#include "Vertex.h"
#include "Material.h"

struct SubMesh {
	std::string name;
	std::vector<MaterialGroup> groups;
	SubMesh();
};
