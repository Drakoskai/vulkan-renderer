#include "VulkanDrawable.h"

#include "VulkanUtil.h"
#include "VulkanRenderSystem.h"
#include "VulkanTexture.h"


namespace Vulkan {

	VulkanDrawable::VulkanDrawable() : numIndices_(0) {}

	VulkanDrawable::~VulkanDrawable() {}

	void VulkanDrawable::Generate(std::vector<SubMesh>& subMeshes) {
		for (auto submesh : subMeshes) {
			uint64_t materialHash = submesh.material.HashCode();
			if (Material::materials.find(materialHash) == end(Material::materials)) {
				Material::materials[materialHash] = submesh.material;
			}
		}

		for (auto submesh : subMeshes) {
			//vertexBuffer_.AddGeometry(submesh, pipeline);

		}
	}

}
