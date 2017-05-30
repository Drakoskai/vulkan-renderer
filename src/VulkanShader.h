#ifndef VULKAN_SHADER_H__
#define VULKAN_SHADER_H__

#include <vulkan/vulkan.h>
#include <vector>
#include "VkCom.h"
#include "GfxTypes.h"

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
#endif
