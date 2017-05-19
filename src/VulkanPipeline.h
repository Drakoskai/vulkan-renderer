#pragma once

#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "Material.h"
#include "VulkanShader.h"

namespace Vulkan {
	class VulkanDrawable;
	class VulkanRenderer;

	struct VulkanPipeline {
		VulkanRenderer* pRenderer;
		VkCom<VkPipeline> pPipeline;
		VkCom<VkPipelineLayout> pipelineLayout;

		VulkanPipeline();
		VulkanPipeline(VulkanRenderer* renderer);
		void SetRenderDevice(VulkanRenderer* renderer);
		void CreatePipeline(VkCom<VkPipeline>& pipeline, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, const std::vector<VkVertexInputBindingDescription>& bindingDescription, const std::vector<VkDescriptorSetLayout>& layouts, const ShaderId shaderid);
	};
}
