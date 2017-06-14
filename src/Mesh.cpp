#include "Mesh.h"
#include "ObjFileLoader.h"
#include "VulkanRenderSystem.h"

Mesh::Mesh() : nextSubMesh_(0) {}

Mesh::~Mesh() {}

void Mesh::LoadFromFile(const std::string& filename) {
	FileFormats::LoadObjFile(filename, *this);
}

size_t Mesh::GetSubMeshCount() const {
	return subMeshes_.size();
}

const std::string& Mesh::GetSubMeshName(uint32_t index) const {
	return subMeshes_[index < subMeshes_.size() ? index : 0].name;
}

SubMesh* Mesh::AddSubMesh() {
	if (nextSubMesh_ == subMeshes_.size()) {
		size_t size = subMeshes_.size();
		subMeshes_.resize(size + 1);
	}

	auto subMesh = &subMeshes_[nextSubMesh_];
	nextSubMesh_++;

	return subMesh;
}

std::vector<SubMesh>& Mesh::GetSubMeshes() { return subMeshes_; }

void Mesh::Generate() {
	Vulkan::VulkanRenderSystem::GetDrawable()->Generate(subMeshes_);
}
