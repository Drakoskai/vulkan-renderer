#include "Mesh.h"
#include "ObjFileLoader.h"
#include "VulkanRenderer.h"

Mesh::Mesh() : nextSubMesh_(0), pRenderer_(nullptr) {}

Mesh::Mesh(Vulkan::VulkanRenderer* renderer) : nextSubMesh_(0), pRenderer_(renderer) {}

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
	if (!pRenderer_) { return; }
	pRenderer_->GetDrawable()->Generate(subMeshes_);
	pRenderer_->RecreateSwapChain();
}
