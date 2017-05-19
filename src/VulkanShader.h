#pragma once

#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "GraphicsStructs.h"
#include <vector>

namespace Vulkan
{
	class VulkanRenderer;
	struct VulkanShader {
		VulkanRenderer* pRenderer_;
		VkCom<VkShaderModule> vertShaderModule_;
		VkCom<VkShaderModule> fragShaderModule_;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages_;

		VulkanShader();
		VulkanShader(VulkanRenderer* pRenderer);
		void SetRenderDevice(VulkanRenderer* renderer) { pRenderer_ = renderer; }
		void LoadShaders(ShaderId shaderId);
	};
}
