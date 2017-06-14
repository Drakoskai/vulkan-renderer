#ifndef VULKAN_PIPELINE_H_
#define VULKAN_PIPELINE_H_

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "VkCom.h"
#include "VulkanShader.h"

namespace Vulkan {
	class VulkanPipeline {
	public:
		static VulkanPipeline* GetPipeline(size_t pipelinehash);
		static void DestroyPipelines();
		
		VulkanPipeline();
		~VulkanPipeline();
		void CreatePipeline(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, const ShaderId shaderid);
		const VkCom<VkPipeline>& GetPipeline() const { return pipeline_; }
		const VkCom<VkPipelineLayout>& GetPipelineLayout() const { return pipelineLayout_; }
		const VkCom<VkDescriptorSetLayout>& GetDescriptorSetLayout() const { return descriptorSetLayout_; }
		bool IsGenerated() const { return isGenerated_; }
		size_t HashCode() const { return 0L; }
	private:
		static std::unordered_map<size_t, VulkanPipeline> pipelines_;

		void CreateDescriptorSetLayout();
		VkCom<VkDescriptorSetLayout> descriptorSetLayout_;
		VkCom<VkPipelineLayout> pipelineLayout_;
		VkCom<VkPipeline> pipeline_;
		bool isGenerated_;
		VkCom<VkDescriptorPool> descriptorPool_;		
	};
}
#endif
