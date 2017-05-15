#pragma once

#include "Mesh.h"
#include "Material.h"

namespace FileFormats {
	void LoadObjFile(const std::string& file, Mesh& mesh);
	void LoadObjFile(const std::string& file, std::vector<VertexPTC>& vertices, std::vector<uint32_t>& indices, std::vector<Material>& materials);
}
