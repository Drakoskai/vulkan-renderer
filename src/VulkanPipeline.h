#ifndef VULKAN_PIPELINE_H_
#define VULKAN_PIPELINE_H_

#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "Material.h"
#include "VulkanShader.h"

namespace Vulkan {
	struct VulkanVertexBuffer;
	class VulkanDrawable;
	class VulkanRenderer;

	struct VulkanPipeline {
		VulkanRenderer* pRenderer;
		VkCom<VkDescriptorSetLayout> descriptorSetLayout;
		VkCom<VkPipelineLayout> pipelineLayout;
		VkCom<VkPipeline> pipeline;
		VulkanPipeline();
		VulkanPipeline(VulkanRenderer* renderer);
		void SetRenderDevice(VulkanRenderer* renderer);
		void CreatePipeline(const VulkanVertexBuffer& vertexBuffer, const ShaderId shaderid);
		void CreateDescriptorSetLayout();
		uint64_t PipelineHash() { return 0L; }
	};
}
#endif
