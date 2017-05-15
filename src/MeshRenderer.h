#pragma once

#include "Component.h"
#include "KaiMath.h"


class Mesh;
class Material;
namespace Vulkan {
	class VulcanRenderer;
}
class MeshRenderer {
public:
	class GameObject* GetGameObject() const;
	MeshRenderer() = default;
	void SetMaterial(Material* material, uint32_t subMeshIndex);
	void SetMesh(Mesh* mesh);
private:
	friend class GameObject;
	friend class Scene;
	static int Type() { return 5; }
	static uint32_t New();
	static MeshRenderer* Get(uint32_t index);
	GameObject* pObj = nullptr;
	Mesh* pMesh = nullptr;
	Vulkan::VulcanRenderer* pRenderer = nullptr;
	std::vector<Material*> mMaterials;

};

namespace Components {
	static ComponentFactory<MeshRenderer> MeshRendererComponents;
}
