#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include "SubMesh.h"

class Mesh {
public:
	Mesh();
	~Mesh();
	void LoadFromFile(const std::string& filename);
	uint32_t GetSubMeshCount() const;
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
