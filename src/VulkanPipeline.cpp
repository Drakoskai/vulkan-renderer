#include "VulkanPipeline.h"
#include "VulkanRenderSystem.h"
#include "VulkanTypes.h"

namespace Vulkan {
	std::unordered_map<size_t, VulkanPipeline> VulkanPipeline::pipelines_;

	VulkanPipeline* VulkanPipeline::GetPipeline(size_t pipelinehash) {
		if (pipelines_.find(pipelinehash) == end(pipelines_)) {
			VulkanPipeline pipeline;
			pipelines_[pipelinehash] = pipeline;
		}
		return &pipelines_[pipelinehash];
	}

	void VulkanPipeline::DestroyPipelines() {
		pipelines_.clear();
	}

	VulkanPipeline::VulkanPipeline() : isGenerated_(false) {}

	VulkanPipeline::~VulkanPipeline() {}

	void VulkanPipeline::CreateDescriptorSetLayout() {
		
		VkDescriptorSetLayoutBinding uboLayoutBinding;
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding diffuseSamplerLayoutBinding;
		diffuseSamplerLayoutBinding.binding = 1;
		diffuseSamplerLayoutBinding.descriptorCount = 1;
		diffuseSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		diffuseSamplerLayoutBinding.pImmutableSamplers = nullptr;
		diffuseSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding normalLayoutBinding;
		normalLayoutBinding.binding = 2;
		normalLayoutBinding.descriptorCount = 1;
		normalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalLayoutBinding.pImmutableSamplers = nullptr;
		normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, diffuseSamplerLayoutBinding, normalLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(VulkanRenderSystem::Device, &layoutInfo, nullptr, descriptorSetLayout_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanPipeline::CreatePipeline(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, const ShaderId shaderid) {
		pipelineLayout_ = { VulkanRenderSystem::Device, vkDestroyPipelineLayout };
		pipeline_ = { VulkanRenderSystem::Device, vkDestroyPipeline };
		descriptorSetLayout_ = { VulkanRenderSystem::Device, vkDestroyDescriptorSetLayout };
		CreateDescriptorSetLayout();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.pNext = nullptr;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		VkDynamicState dynamicStateEnables[2];
		dynamicStateEnables[0] = VK_DYNAMIC_STATE_VIEWPORT;
		dynamicStateEnables[1] = VK_DYNAMIC_STATE_SCISSOR;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = &dynamicStateEnables[0];
		dynamicState.dynamicStateCount = 2;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
		std::vector<VkPushConstantRange> pushconstatRanges = {
				VkPushConstantRange {
					VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(UniformMaterialObject)
				}
		};
		pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushconstatRanges.size());
		pipelineLayoutInfo.pPushConstantRanges = pushconstatRanges.data();

		if (vkCreatePipelineLayout(VulkanRenderSystem::Device, &pipelineLayoutInfo, nullptr, pipelineLayout_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(VulkanShader::GetShader(shaderid)->Size());
		pipelineInfo.pStages = VulkanShader::GetShader(shaderid)->CreateInfo();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout_;
		pipelineInfo.renderPass = VulkanRenderSystem::RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDynamicState = &dynamicState;


		if (vkCreateGraphicsPipelines(VulkanRenderSystem::Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pipeline_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		isGenerated_ = true;
	}
}
