#ifndef SUB_MESH_H_
#define SUB_MESH_H_

#include "Vertex.h"
#include "Material.h"

struct SubMesh {
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	Material material;
	SubMesh();
};
#endif
