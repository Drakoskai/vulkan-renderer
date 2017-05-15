#include "VulkanShader.h"
#include "VkCom.h"
#include "Util.h"
#include "VulkanRenderer.h"

namespace Vulkan
{

	VulkanShader::VulkanShader() : pRenderer(nullptr) {}

	VulkanShader::VulkanShader(VulkanRenderer* pRenderer) : pRenderer(pRenderer) {}

	void VulkanShader::LoadShaders(ShaderId shaderId) {
		if (!pRenderer) {
			throw std::runtime_error("Vulkan Shader not intialized!");
		}

		auto vertShaderCode = ReadFile(shaderId.GetVertexShader());
		auto fragShaderCode = ReadFile(shaderId.GetFragmentShader());

		vertShaderModule = { pRenderer->device, vkDestroyShaderModule };
		fragShaderModule = { pRenderer->device, vkDestroyShaderModule };
		pRenderer->CreateShaderModule(vertShaderCode, vertShaderModule);
		pRenderer->CreateShaderModule(fragShaderCode, fragShaderModule);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		shaderStages[0] = vertShaderStageInfo;
		shaderStages[1] = fragShaderStageInfo;
	}
}
