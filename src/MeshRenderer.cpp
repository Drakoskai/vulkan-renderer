#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Util.h"

GameObject* MeshRenderer::GetGameObject() const { return pObj; }

void MeshRenderer::SetMaterial(Material* material, uint32_t subMeshIndex) {
	if (subMeshIndex >= 0 && subMeshIndex < uint32_t(mMaterials.size())) {
		mMaterials[subMeshIndex] = material;
	}
}

void MeshRenderer::SetMesh(Mesh* mesh) {
	SafeDelete(pMesh);
	pMesh = mesh;
	if (mesh != nullptr) {
		mMaterials.resize(mesh->GetSubMeshes().size());
		pMesh->Generate();
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
