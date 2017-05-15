#include "Mesh.h"
#include <unordered_map>
#include "ObjFileLoader.h"
#include "VulkanRenderer.h"

Mesh::Mesh()
	: m_aabbMin(Vec3(MaxFloatValue, MaxFloatValue, MaxFloatValue)),
	m_aabbMax(Vec3(-MaxFloatValue, -MaxFloatValue, -MaxFloatValue)), mNextSubMesh(0), pRenderer(nullptr) {
}

Mesh::Mesh(Vulkan::VulkanRenderer* renderer) : mNextSubMesh(0), pRenderer(renderer) {}

Mesh::~Mesh() {}

void Mesh::LoadFromFile(const std::string& path, const std::string& filename) {
	FileFormats::LoadObjFile(path + filename, path, *this);
}

const Vec3& Mesh::GetAABBMin() const { return m_aabbMin; }

const Vec3& Mesh::GetAABBMax() const { return m_aabbMax; }

const Vec3& Mesh::GetSubMeshAABBMin(uint32_t subMeshIndex) const {
	return m_subMeshes[subMeshIndex < m_subMeshes.size() ? subMeshIndex : 0].aabbMin;
}

const Vec3& Mesh::GetSubMeshAABBMax(uint32_t subMeshIndex) const {
	return m_subMeshes[subMeshIndex < m_subMeshes.size() ? subMeshIndex : 0].aabbMax;
}

size_t Mesh::GetSubMeshCount() const {
	return m_subMeshes.size();
}

const std::string& Mesh::GetSubMeshName(uint32_t index) const {
	return m_subMeshes[index < m_subMeshes.size() ? index : 0].name;
}

SubMesh* Mesh::AddSubMesh() {
	if (mNextSubMesh == m_subMeshes.size()) {
		size_t size = m_subMeshes.size();
		m_subMeshes.resize(size + 1);
	}

	auto subMesh = &m_subMeshes[mNextSubMesh];
	subMesh->SetRenderDevice(pRenderer);

	mNextSubMesh++;

	return subMesh;
}

std::vector<SubMesh>& Mesh::GetSubMeshes() { return m_subMeshes; }

void Mesh::SolveAABB() {
	for (auto submesh : m_subMeshes) {
		submesh.SolveAABB();
		m_aabbMin = Min(m_aabbMin, submesh.aabbMin);
		m_aabbMax = Max(m_aabbMax, submesh.aabbMax);
	}
}

void Mesh::Generate() {
	if (!pRenderer) { return; }
	for (auto& submesh : m_subMeshes) {
		submesh.Generate();
	}
	pRenderer->RecreateSwapChain();
}
