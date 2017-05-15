#pragma once

#include "KaiMath.h"
#include <vector>
#include "SubMesh.h"

class Mesh {
public:
	Mesh();
	Mesh(Vulkan::VulkanRenderer* renderer);
	~Mesh();
	void SetRenderDevice(Vulkan::VulkanRenderer* renderer) { pRenderer = renderer; }
	void LoadFromFile(const std::string& path, const std::string& filename);
	const Vec3& GetAABBMin() const;
	const Vec3& GetAABBMax() const;
	const Vec3& GetSubMeshAABBMin(uint32_t subMeshIndex) const;
	const Vec3& GetSubMeshAABBMax(uint32_t subMeshIndex) const;
	size_t GetSubMeshCount() const;
	const std::string& GetSubMeshName(uint32_t index) const;
	SubMesh* AddSubMesh();
	std::vector<SubMesh>& GetSubMeshes();
	void SolveAABB();
	void Generate();

private:
	const float MaxFloatValue = 99999999.0f;
	friend class MeshRenderer;
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;

	std::vector<SubMesh> m_subMeshes;
	uint32_t mNextSubMesh;

	Vulkan::VulkanRenderer* pRenderer;
};
