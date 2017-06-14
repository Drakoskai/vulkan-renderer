#ifndef MESH_H__
#define MESH_H__

#include <vector>
#include "SubMesh.h"
#include "VulkanShader.h"

class Mesh {
public:
	Mesh();
	~Mesh();
	void LoadFromFile(const std::string& filename);
	size_t GetSubMeshCount() const;
	const std::string& GetSubMeshName(uint32_t index) const;
	SubMesh* AddSubMesh();
	std::vector<SubMesh>& GetSubMeshes();
	void Generate();

private:
	friend class MeshRenderer;
	std::vector<SubMesh> subMeshes_;
	uint32_t nextSubMesh_;
};
#endif
