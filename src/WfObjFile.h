#pragma once

#include "pch.h"
#include <vector>
#include "Vertex.h"
#include <map>
#include "MeshBuilder.h"

class ObjFile {
public:
	ObjFile(const std::string& path);
	~ObjFile();
	
	const std::vector<PreMesh>& LoadData();

private:
	void ReadIndices();
	bool m_hasVNormals;
	bool m_hasTexCoord;
	std::string m_filename;

	std::vector<Vec3> m_vertex;
	std::vector<Vec3> m_vnormal;
	std::vector<Vec2> m_tcoord;

	std::map<int32_t, int32_t> m_vertIdxs;
	std::map<int32_t, int32_t> m_normIdxs;
	std::map<int32_t, int32_t> m_uvIdxs;
	std::vector<PreMesh> m_meshes;
};
