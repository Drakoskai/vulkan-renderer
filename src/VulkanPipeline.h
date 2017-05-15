#pragma once

#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "VulkanShader.h"



namespace Vulkan {
	class VulkanDrawable;
	class VulkanRenderer;

	struct VulkanPipeline {
		VulkanRenderer* pRenderer;
		VulkanShader shaderProgram;	
		VkCom<VkPipelineCache> pipelineCache;	
		VkCom<VkPipelineLayout> pipelineLayout;

		VulkanPipeline();
		VulkanPipeline(VulkanRenderer* renderer);
		void SetRenderDevice(VulkanRenderer* renderer);
		void CreatePipelineCache();
		void CreatePipeline(VkCom<VkPipeline>& pipeline, VulkanDrawable* drawable);
	};
}
