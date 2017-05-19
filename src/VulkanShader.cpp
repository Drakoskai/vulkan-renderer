#include "VulkanShader.h"
#include "VkCom.h"
#include "Util.h"
#include "VulkanRenderer.h"

namespace Vulkan
{
	VulkanShader::VulkanShader() : pRenderer_(nullptr) {}

	VulkanShader::VulkanShader(VulkanRenderer* pRenderer) : pRenderer_(pRenderer) {}

	void VulkanShader::LoadShaders(ShaderId shaderId) {
		if (!pRenderer_) {
			throw std::runtime_error("Vulkan Shader not intialized!");
		}

		auto vertShaderCode = ReadFile(shaderId.GetVertexShader());
		auto fragShaderCode = ReadFile(shaderId.GetFragmentShader());

		vertShaderModule_ = { pRenderer_->device_, vkDestroyShaderModule };
		fragShaderModule_ = { pRenderer_->device_, vkDestroyShaderModule };
		pRenderer_->CreateShaderModule(vertShaderCode, vertShaderModule_);
		pRenderer_->CreateShaderModule(fragShaderCode, fragShaderModule_);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule_;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule_;
		fragShaderStageInfo.pName = "main";

		shaderStages_.push_back(vertShaderStageInfo);
		shaderStages_.push_back(fragShaderStageInfo);
	}
}
