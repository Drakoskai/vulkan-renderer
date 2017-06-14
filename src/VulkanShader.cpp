#include "VulkanShader.h"
#include "VkCom.h"
#include "Util.h"
#include "VulkanRenderSystem.h"

namespace Vulkan
{
	std::unordered_map<ShaderId, VulkanShader> VulkanShader::shadercache_;

	VulkanShader::VulkanShader() {}

	VulkanShader::~VulkanShader() {}

	void VulkanShader::LoadShaders(ShaderId shaderId) {
		vertShaderModule_ = { VulkanRenderSystem::Device, vkDestroyShaderModule };
		fragShaderModule_ = { VulkanRenderSystem::Device, vkDestroyShaderModule };

		auto vertShaderCode = ReadFile(shaderId.GetVertexShader());
		auto fragShaderCode = ReadFile(shaderId.GetFragmentShader());

		VulkanRenderSystem::CreateShaderModule(vertShaderCode, vertShaderModule_);
		VulkanRenderSystem::CreateShaderModule(fragShaderCode, fragShaderModule_);

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

	uint32_t VulkanShader::Size() const {
		return static_cast<uint32_t>(shaderStages_.size());
	}

	VkPipelineShaderStageCreateInfo* VulkanShader::CreateInfo() {
		return shaderStages_.data();
	}

	VulkanShader* VulkanShader::GetShader(ShaderId id) {
		if (shadercache_.find(id) == end(shadercache_)) {
			VulkanShader s;
			shadercache_[id] = s;
			shadercache_[id].LoadShaders(id);
		}

		return &shadercache_[id];
	}
}
