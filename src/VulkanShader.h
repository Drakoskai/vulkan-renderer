#pragma once
#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "GraphicsStructs.h"

namespace Vulkan
{
	class VulkanRenderer;
	struct VulkanShader {
		VulkanRenderer* pRenderer;
		VkCom<VkShaderModule> vertShaderModule;
		VkCom<VkShaderModule> fragShaderModule;
		VkPipelineShaderStageCreateInfo shaderStages[2];

		VulkanShader();
		explicit VulkanShader(VulkanRenderer* pRenderer);
		void SetRenderDevice(VulkanRenderer* renderer) { pRenderer = renderer; }
		void LoadShaders(ShaderId shaderId);

	};

}
