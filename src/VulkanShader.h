#ifndef VULKAN_SHADER_H__
#define VULKAN_SHADER_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include "VkCom.h"
#include "GfxTypes.h"


namespace Vulkan {
	class VulkanShader {
	public:
		static VulkanShader* GetShader(ShaderId id);

		VulkanShader();
		~VulkanShader();
		void LoadShaders(ShaderId shaderId);
		uint32_t Size() const;
		VkPipelineShaderStageCreateInfo* CreateInfo();
	private:
		static std::unordered_map<ShaderId, VulkanShader> shadercache_;

		VkCom<VkShaderModule> vertShaderModule_;
		VkCom<VkShaderModule> fragShaderModule_;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages_;	
	};
}
#endif
