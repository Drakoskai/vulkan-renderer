#include "VulkanRenderProgram.h"
#include "Util.h"
#include "VulkanTypes.h"
#include "VulkanUtil.h"
#include "VulkanTexture.h"
#include <assert.h>
#include "Material.h"
#include "VulkanInit.h"

namespace Vulkan {
	
	static void CreateVertexDescriptions() {
		VertexLayout drawVertext{};
		drawVertext.attributeDescriptions = Vertex::GetAttributeDescriptions();
		drawVertext.bindingDescriptions = Vertex::GetBindingDescription();
		vertexLayouts[DrawVertex] = drawVertext;
	}

	static void CreateDescriptorPools(VkDescriptorPool(&descriptorPools)[NumberOfFrameBuffers]) {
		const uint32_t numPools = NumberOfFrameBuffers;
		std::array<VkDescriptorPoolSize, numPools> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = MaxUniformDescriptors;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = MaxSamplersDescriptors;

		VkDescriptorPoolCreateInfo poolInfo;
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = numPools;
		poolInfo.pNext = nullptr;
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = MaxDesciptorSets;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		for (int i = 0; i < NumberOfFrameBuffers; i++) {
			if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &descriptorPools[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}
	}

	static void CreateDescriptorSetLayout(const VulkanShader& vertexShader, const VulkanShader& fragmentShader, VulkanRenderProgram& renderProg) {
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayoutBinding binding = {};
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uint32_t bindingId = 0;
		for (const ShaderBindingType shaderBinding : vertexShader.bindings) {
			binding.binding = bindingId++;
			binding.descriptorType = GetDescriptorType(shaderBinding);
			renderProg.mBindings.push_back(shaderBinding);
			bindings.push_back(binding);
		}

		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		for (const ShaderBindingType shaderBinding : fragmentShader.bindings) {
			binding.binding = bindingId++;
			binding.descriptorType = GetDescriptorType(shaderBinding);
			renderProg.mBindings.push_back(shaderBinding);
			bindings.push_back(binding);
			if (shaderBinding == Sampler) {
				for (const auto pair : Material::materials) {
					Material material = pair.second;
					if (material.shader == fragmentShader.shaderid) {
						renderProg.mImageIds.push_back(material.diffuseTexture);
					}
				}
			}
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &renderProg.hDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &renderProg.hDescriptorSetLayout;
		if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &renderProg.hPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	static VkPipeline CreatePipeline(uint64_t pipelineState, VertexType vertexType, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkPipelineLayout pipelineLayout) {
		PrintPipelineState(pipelineState);
		VertexLayout& vertexLayout = vertexLayouts[vertexType];
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexLayout.bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = vertexLayout.bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexLayout.attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexLayout.attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;// GetPolygonMode(pipelineState); //VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;// GetCullMode(pipelineState); //VK_CULL_MODE_FRONT_BIT
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE; //(pipelineState & DepthFunction) == DepthFunctionNoneWriteOff ? VK_TRUE : VK_FALSE; // VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;// (pipelineState & DepthFunction) == DepthFunctionLessOrEqualWriteOn ? VK_TRUE : VK_FALSE;// VK_TRUE;
		depthStencil.depthCompareOp = GetDepthCompareOp(pipelineState);// VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;// GetBlendMode(pipelineState); //VK_FALSE;

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

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		VkDynamicState dynamicStateEnables[2];
		dynamicStateEnables[0] = VK_DYNAMIC_STATE_VIEWPORT;
		dynamicStateEnables[1] = VK_DYNAMIC_STATE_SCISSOR;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = &dynamicStateEnables[0];
		dynamicState.dynamicStateCount = 2;

		std::array<VkPipelineShaderStageCreateInfo, 2> stages;
		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].pName = "main";
		stages[0].module = vertexShader;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].pName = "main";
		stages[1].module = fragmentShader;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
		pipelineInfo.pStages = stages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = context.renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDynamicState = &dynamicState;

		VkPipeline pipeline = VK_NULL_HANDLE;

		VkCheckOrThrow(vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "failed to create graphics pipeline!");
		
		return pipeline;
	}

	VkPipeline VulkanRenderProgram::GetPipeline(uint64_t pipelineState, VkShaderModule vertexShader, VkShaderModule fragmentShader) {
		for (const VulkanPipeline pipelineStateObject : mPipelines) {
			if (pipelineStateObject.mPipelineState == pipelineState) {
				return pipelineStateObject.hPipeline;
			}
		}

		const VkPipeline pipeline = CreatePipeline(pipelineState, mVertexType, vertexShader, fragmentShader, hPipelineLayout);
		VulkanPipeline pipelineStateObject;
		pipelineStateObject.mPipelineState = pipelineState;
		pipelineStateObject.hPipeline = pipeline;
		mPipelines.push_back(pipelineStateObject);

		return pipeline;
	}

	VulkanGpuProgramState::~VulkanGpuProgramState() {}

	void VulkanGpuProgramState::Init() {
		LoadShader(DefaultUnlitShader);

		mRenderProgs.resize(1);
		VulkanRenderProgram& prog = mRenderProgs[0];

		prog.mVertexShaderIdx = 0;
		prog.mFragmentShaderIdx = 1;
		prog.mName = "default";
		prog.mVertexType = DrawVertex;

		CreateDescriptorSetLayout(mShaders[0], mShaders[1], prog);
		CreateVertexDescriptions();
		CreateDescriptorPools(hDescriptorPools);

		const VkDeviceSize uboSize = MaxDesciptorSets * MaxDescriptorSetUniforms * sizeof(float) * 4;

		for (int i = 0; i < NumberOfFrameBuffers; ++i) {
			pUniformBuffer[i] = new UniformBuffer();
			pUniformBuffer[i]->AllocateBufferObject(nullptr, uboSize, Dynamic);
		}
	}

	void VulkanGpuProgramState::Shutdown() {
		for (VulkanRenderProgram renderProg : mRenderProgs) {
			for (const VulkanPipeline pipelineStateObject : renderProg.mPipelines) {
				vkDestroyPipeline(context.device, pipelineStateObject.hPipeline, nullptr);
			}
			renderProg.mPipelines.clear();
			vkDestroyDescriptorSetLayout(context.device, renderProg.hDescriptorSetLayout, nullptr);
			vkDestroyPipelineLayout(context.device, renderProg.hPipelineLayout, nullptr);
		}
		mRenderProgs.clear();

		for (const VulkanShader shader : mShaders) {
			vkDestroyShaderModule(context.device, shader.shaderModule, nullptr);
		}
		mShaders.clear();

		for (VkDescriptorPool descriptorPool : hDescriptorPools) {
			vkResetDescriptorPool(context.device, descriptorPool, 0);
			vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
		}
		memset(hDescriptorSets, 0, sizeof(hDescriptorSets));
		memset(hDescriptorPools, 0, sizeof(hDescriptorPools));
		vertexLayouts.clear();
	}

	void VulkanGpuProgramState::Start() {
		mCounter++;
		mCurrentBuffer = mCounter % NumberOfFrameBuffers;
		mCurrentDescriptorSet = 0;
		mCurrentUniformBufferOffset = 0;
		vkResetDescriptorPool(context.device, hDescriptorPools[mCurrentBuffer], 0);
	}

	void VulkanGpuProgramState::Bind(int index) {
		if (mCurrent == index) {
			return;
		}
		mCurrent = index;
	}

	void VulkanGpuProgramState::Commit(uint64_t pipelinestate) {
		VulkanRenderProgram prog = mRenderProgs[mCurrent];
		const int vertexShaderidx = prog.mVertexShaderIdx;
		const int fragmentShaderidx = prog.mFragmentShaderIdx;

		const VkPipeline pipeline = prog.GetPipeline(pipelinestate, mShaders[vertexShaderidx].shaderModule, fragmentShaderidx != -1 ? mShaders[fragmentShaderidx].shaderModule : VK_NULL_HANDLE);

		CreateDescriptorSet(prog);

		VkDescriptorSet descriptorSet = hDescriptorSets[mCurrentBuffer][mCurrentDescriptorSet];
		mCurrentDescriptorSet++;
		int writeIndex = 0;
		int bufferIndex = 0;
		int	imageIndex = 0;
		int bindingIndex = 0;

		VkWriteDescriptorSet writes[MaxDescriptorSetWrites];
		UniformBuffer * ubos[2] = { nullptr, nullptr };
		UniformBuffer vertUbo;

		if (vertexShaderidx > -1 && mShaders[vertexShaderidx].params.size() > 0) {
			AllocateUniformBlock(mShaders[vertexShaderidx], vertUbo);
			ubos[0] = &vertUbo;
		}
		UniformBuffer fragUbo;
		if (fragmentShaderidx > -1 && mShaders[fragmentShaderidx].params.size() > 0) {
			AllocateUniformBlock(mShaders[fragmentShaderidx], fragUbo);
			ubos[1] = &fragUbo;
		}
		for (int i = 0; i < prog.mBindings.size(); ++i) {
			const ShaderBindingType bindingType = prog.mBindings[i];
			switch (bindingType) {
			case Uniform:
			{
				UniformBuffer * ubo = ubos[bufferIndex++];
				VkDescriptorBufferInfo bufferInfo;
				memset(&bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
				bufferInfo.buffer = ubo->Buffer();
				bufferInfo.offset = ubo->GetOffset();
				bufferInfo.range = ubo->GetSize();

				VkWriteDescriptorSet & write = writes[writeIndex++];
				memset(&write, 0, sizeof(VkWriteDescriptorSet));
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descriptorSet;
				write.dstBinding = bindingIndex++;
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.pBufferInfo = &bufferInfo;

				break;
			}
			case Sampler:
			{
				VulkanTexture* diffuseTexture = VulkanTexture::GetTexture(prog.mImageIds[imageIndex++]);
				VkWriteDescriptorSet & write = writes[writeIndex++];

				memset(&write, 0, sizeof(VkWriteDescriptorSet));
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descriptorSet;
				write.dstBinding = bindingIndex++;
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.pImageInfo = &diffuseTexture->GetImageInfo();

				break;
			}
			case PushConstant:;
			default: ;
			}
		}

		vkUpdateDescriptorSets(context.device, writeIndex, writes, 0, nullptr);
		vkCmdBindDescriptorSets(context.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, prog.hPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		vkCmdBindPipeline(context.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void VulkanGpuProgramState::AllocateUniformBlock(const VulkanShader& shader, UniformBuffer& ubo) {
		const int bytes = shader.GetInputStageSizeBytes();
		ubo.Reference(*pUniformBuffer[mCurrentBuffer], mCurrentUniformBufferOffset, bytes);
		void* data = ubo.Map();
		memcpy(data, mUniformData[shader.uniformIdx], sizeof(bytes));
		ubo.Unmap();

		mCurrentUniformBufferOffset += bytes;
	}

	void VulkanGpuProgramState::LoadShader(ShaderId shaderId) {
		const auto vertShaderCode = ReadFile(shaderId.GetVertexShader());
		const auto fragShaderCode = ReadFile(shaderId.GetFragmentShader());

		VulkanShader vert;
		vert.name = shaderId.GetVertexShader();
		vert.shaderid = shaderId;
		vert.stage = ShaderVertex;
		vert.uniformIdx = static_cast<uint32_t>(mShaders.size());
		VulkanShader::CreateShaderModule(vertShaderCode, vert);
		BuildShaderParams(ReadSpirv(shaderId.GetVertexShader().c_str()), vert);
		mShaders.push_back(vert);

		VulkanShader frag;
		frag.name = shaderId.GetFragmentShader();
		frag.shaderid = shaderId;
		frag.uniformIdx = static_cast<uint32_t>(mShaders.size());
		frag.stage = ShaderFragment;
		VulkanShader::CreateShaderModule(fragShaderCode, frag);
		BuildShaderParams(ReadSpirv(shaderId.GetFragmentShader().c_str()), frag);
		mShaders.push_back(frag);
	}

	void VulkanGpuProgramState::CreateDescriptorSet(const VulkanRenderProgram& prog) {
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = hDescriptorPools[mCurrentBuffer];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &prog.hDescriptorSetLayout;

		if (vkAllocateDescriptorSets(context.device, &allocInfo, &hDescriptorSets[mCurrentBuffer][mCurrentDescriptorSet]) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}
	}
}
