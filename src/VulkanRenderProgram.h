#ifndef VULKAN_PIPELINE_H_
#define VULKAN_PIPELINE_H_

#include "stdafx.h"
#include "Vertex.h"
#include "VulkanTypes.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include <unordered_map>

namespace Vulkan {

	struct VertexLayout {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};

	static std::unordered_map<VertexType, VertexLayout> vertexLayouts;

	inline uint64_t GetDefaultPipelineState() {
		PipelineStateBuilder builder;
		builder.SetPrimitiveTopology(PrimitiveTopologyTriangles);
		builder.SetCullMode(CullModeFront);
		builder.SetFillMode(FillModeSolid);
		return builder.pipelineState;
	}

	struct VulkanPipeline {
		uint64_t mPipelineState;
		VkPipeline hPipeline;
		VulkanPipeline() : mPipelineState(GetDefaultPipelineState()), hPipeline(VK_NULL_HANDLE) {}
	};

	struct VulkanRenderProgram {
		VkPipeline GetPipeline(uint64_t pipelineState, VkShaderModule vertexShader, VkShaderModule fragmentShader);
		std::string mName;
		int mCurrent;
		int mVertexShaderIdx;
		int mFragmentShaderIdx;
		VertexType mVertexType;
		VkPipelineLayout hPipelineLayout;
		VkDescriptorSetLayout hDescriptorSetLayout;
		std::vector<VulkanPipeline> mPipelines;
		std::vector<ShaderBindingType> mBindings;
		std::vector<TextureId> mImageIds;

		VulkanRenderProgram() : mCurrent(-1), mVertexShaderIdx(-1), mFragmentShaderIdx(-1), mVertexType(DrawVertex),
			hPipelineLayout(VK_NULL_HANDLE), hDescriptorSetLayout(VK_NULL_HANDLE) {}
	};

	class VulkanGpuProgramState {
	public:
		VulkanGpuProgramState() : mCounter(0), mCurrent(0), mCurrentBuffer(0), mCurrentDescriptorSet(0),
			mCurrentUniformBufferOffset(0) {
			mUniformData.resize(NumberOfFrameBuffers);
		}

		~VulkanGpuProgramState();
		void Init();
		void Shutdown();
		void Start();
		void Bind(int index);
		void Commit(uint64_t pipelinestate);
		void SetUniformData(uint32_t index, void* data) { mUniformData[index] = data; }
		const VulkanRenderProgram& GetCurrentRenderProg() const { return mRenderProgs[mCurrent]; }
	private:
		void CreateDescriptorSet(const VulkanRenderProgram& prog);
		void AllocateUniformBlock(const VulkanShader& shader, UniformBuffer& ubo);
		void LoadShader(ShaderId shader);
		std::vector<VulkanRenderProgram> mRenderProgs;
		std::vector<VulkanShader> mShaders;
		VkDescriptorPool hDescriptorPools[NumberOfFrameBuffers];
		VkDescriptorSet hDescriptorSets[NumberOfFrameBuffers][MaxDesciptorSets];
		UniformBuffer* pUniformBuffer[NumberOfFrameBuffers];
		VkDeviceMemory hUniformStagingBufferMemory[NumberOfFrameBuffers];
		VkDeviceMemory hUniformBufferMemory[NumberOfFrameBuffers];
		std::vector<void*> mUniformData;
		int mCounter;
		int mCurrent;
		int mCurrentBuffer;
		int mCurrentDescriptorSet;
		int mCurrentUniformBufferOffset;
	};
}
#endif
