#ifndef VULKAN_PIPELINE_H_
#define VULKAN_PIPELINE_H_

#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "VulkanShader.h"

namespace Vulkan {
	class VulkanVertexBuffer;
	class VulkanDrawable;
	class VulkanRenderer;

	class VulkanPipeline {
	public:
		uint64_t static PipelineHash() { return 0L; }
		VulkanPipeline();
		VulkanPipeline(VulkanRenderer* renderer);
		~VulkanPipeline();
		void SetRenderDevice(VulkanRenderer* renderer);
		void CreatePipeline(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, const ShaderId shaderid);
		const VkCom<VkPipeline>& GetPipeline() const { return pipeline_; }
		const VkCom<VkPipelineLayout>& GetPipelineLayout() const { return pipelineLayout_; }
		const VkCom<VkDescriptorSetLayout>& GetDescriptorSetLayout() const { return descriptorSetLayout_; }
		bool IsGenerated() const { return isGenerated_; }
	private:
		void CreateDescriptorSetLayout();
		VulkanRenderer* pRenderer_;
		VkCom<VkDescriptorSetLayout> descriptorSetLayout_;
		VkCom<VkPipelineLayout> pipelineLayout_;
		VkCom<VkPipeline> pipeline_;
		bool isGenerated_;
	};
}
#endif
