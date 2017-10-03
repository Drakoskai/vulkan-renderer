#ifndef VULKAN_DRAWABLE_H__
#define VULKAN_DRAWABLE_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include "VulkanRenderProgram.h"
#include "Material.h"
#include "SubMesh.h"

namespace Vulkan {
	class VulkanDrawable {
	public:
		VulkanDrawable();
		~VulkanDrawable();
		void VulkanDrawable::Generate(std::vector<SubMesh>& subMeshes);
	private:
		VulkanGpuProgramState programState;
		uint32_t numIndices_;
		uint32_t nextDescriptor = 0;
		uint64_t vertHandle;
		uint64_t idxHandle;

		std::unordered_map<uint64_t, VkDescriptorSet> materialDescriptors_;
	};
}
#endif
