#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Util.h"

GameObject* MeshRenderer::GetGameObject() const { return pObj_; }

ComponentFactory<MeshRenderer>& MeshRenderer::GetMeshRenderers() {
	return Components::MeshRendererComponents;
}

void MeshRenderer::SetMaterial(Material* material, uint32_t subMeshIndex) {
	if (subMeshIndex >= 0 && subMeshIndex < uint32_t(materials_.size())) {
		materials_[subMeshIndex] = material;
	}
}

void MeshRenderer::SetMesh(Mesh* mesh) {
	SafeDelete(pMesh_);
	pMesh_ = mesh;
	if (mesh != nullptr) {
		materials_.resize(mesh->GetSubMeshes().size());
		pMesh_->Generate();
	}
}

uint32_t MeshRenderer::New() {
	using namespace Components;
	return MeshRendererComponents.NewHandle();
}

MeshRenderer* MeshRenderer::Get(uint32_t index) {
	using namespace Components;

	return MeshRendererComponents.Get(index);
}
