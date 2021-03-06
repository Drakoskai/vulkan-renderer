#ifndef MESH_RENDERER_H_
#define MESH_RENDERER_H_

#include "Component.h"

class Mesh;
struct Material;

namespace Vulkan {
	class VulcanRenderer;
}

class MeshRenderer {
public:
	class GameObject* GetGameObject() const;
	static ComponentFactory<MeshRenderer>& GetMeshRenderers();
	MeshRenderer() = default;
	void SetMaterial(Material* material, uint32_t subMeshIndex);
	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() { return pMesh_; }
private:
	friend class GameObject;
	friend class Scene;
	static int Type() { return 5; }
	static uint32_t New();
	static MeshRenderer* Get(uint32_t index);
	std::vector<Material*> materials_;
	GameObject* pObj_ = nullptr;
	Mesh* pMesh_ = nullptr;
};

namespace Components {
	static ComponentFactory<MeshRenderer> MeshRendererComponents;
}
#endif
