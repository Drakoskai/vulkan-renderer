#ifndef VULKAN_PIPELINE_H_
#define VULKAN_PIPELINE_H_

#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "Material.h"
#include "VulkanShader.h"

namespace Vulkan {
	class VulkanDrawable;
	class VulkanRenderer;

	struct VertexBufferInfo {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	};

	struct VulkanPipeline {
		VulkanRenderer* pRenderer;
		VkCom<VkDescriptorSetLayout> descriptorSetLayout;
		VkCom<VkPipelineLayout> pipelineLayout;
		VkCom<VkPipeline> pipeline;
		VulkanPipeline();
		VulkanPipeline(VulkanRenderer* renderer);
		void SetRenderDevice(VulkanRenderer* renderer);
		void CreatePipeline(const VertexBufferInfo& vertexInfo, const ShaderId shaderid);
		void CreateDescriptorSetLayout();
	};
}
#endif
