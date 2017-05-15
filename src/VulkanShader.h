#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include "VkCom.h"

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
		void LoadShaders(std::string vertexShaderFile, std::string fragmentShaderFile);
	};

}
