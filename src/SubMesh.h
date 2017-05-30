#ifndef SUB_MESH_H__
#define SUB_MESH_H__

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
