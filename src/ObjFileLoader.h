#pragma once

#include "Mesh.h"
#include "Material.h"

namespace FileFormats {
	void LoadObjFile(const std::string& file, const std::string& materialbase, Mesh& mesh);
}
