#ifndef VULKAN_SHADER_H_
#define VULKAN_SHADER_H_

#include "VulkanTypes.h"
#include "Vertex.h"

namespace Vulkan {

	enum ShaderStage {
		ShaderVertex,
		ShaderCompute,
		ShaderTese,
		ShaderTesc,
		ShaderFragment
	};

	enum ShaderBindingType {
		Uniform,
		PushConstant,
		Sampler
	};

	enum ShaderParamType {
		Float,
		Vector2,
		Vector3,
		Vector4,
		Matrix4,
		None
	};

	struct ShaderResource {
		ShaderBindingType binding;
		ShaderParamType param;
	};

	inline VkDescriptorType GetDescriptorType(ShaderBindingType type) {
		switch (type) {
		case Uniform: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case Sampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	struct VulkanShader {
		VkShaderModule shaderModule;
		ShaderStage stage;
		ShaderId shaderid;
		uint32_t uniformIdx = 0;
		std::string name;
		std::vector<ShaderBindingType> bindings;
		std::vector<ShaderParamType> params;
		VulkanShader() : shaderModule(VK_NULL_HANDLE), stage() {}
		int GetInputStageSizeBytes() const;
		static void CreateShaderModule(const std::vector<char>& code, VulkanShader& shader);
	};

}
#endif
