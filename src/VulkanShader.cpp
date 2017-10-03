#include "VulkanShader.h"
#include <spirv_cross/spirv_glsl.hpp>
#include "VulkanInit.h"
#include "VulkanUtil.h"
#include <memory.h>

namespace Vulkan {
	int VulkanShader::GetInputStageSizeBytes() const {
		VkDeviceSize bytes = 0;
		for (int i = 0; i < params.size(); i++) {
			if (bindings[i] != Uniform) {
				continue;
			}
			const auto param = params[i];
			switch (param) {
			case Float: bytes += sizeof(float);
			case Vector2:
			{
				bytes += sizeof(float) * 2;
				break;
			}
			case Vector3:
			{
				bytes += sizeof(float) * 3;
				break;
			}
			case Vector4:
			{
				bytes += sizeof(float) * 4;
				break;
			}
			case Matrix4:
			{
				bytes += sizeof(float) * 4 * 4;
				break;
			}
			case None:;
			default: break;
			}
		}

		bytes = Align(bytes, context.gpu->physicalDeviceProps.limits.minUniformBufferOffsetAlignment);// ALIGN(bytes, static_cast<int>(context.gpu->physicalDeviceProps.limits.minUniformBufferOffsetAlignment));
		return static_cast<int>(bytes);
	}

	void HandleMatrix(VulkanShader& shader, const spirv_cross::SPIRType& type) {
		switch (type.width) {
		case 4:
		{
			switch (type.columns) {
			case 4:
			{
				shader.bindings.push_back(Uniform);
				shader.params.push_back(Matrix4);
				break;
			}
			default:;
			}
			break;
		}
		default:;
		}
	}

	void HandleVectors(VulkanShader& shader, const uint32_t& vecsize) {
		switch (vecsize) {
		case 1:
		{
			shader.bindings.push_back(Uniform);
			shader.params.push_back(Float);
			break;
		}
		case 2:
		{
			shader.bindings.push_back(Uniform);
			shader.params.push_back(Vector2);
			break;
		}
		case 3:
		{
			shader.bindings.push_back(Uniform);
			shader.params.push_back(Vector3);
			break;
		}
		case 4:
		{
			shader.bindings.push_back(Uniform);
			shader.params.push_back(Vector4);
			break;
		}
		default:;
		}
	}

	void BuildShaderParams(const std::vector<uint32_t>& spirv, VulkanShader& shader) {
		spirv_cross::Compiler comp(move(spirv));
		spirv_cross::ShaderResources resources = comp.get_shader_resources();

		for (auto &p : resources.stage_inputs) {
			const auto type = comp.get_type(p.type_id);
			switch (type.basetype) {
			case spirv_cross::SPIRType::Void: break;
			case spirv_cross::SPIRType::Boolean: break;
			case spirv_cross::SPIRType::Char: break;
			case spirv_cross::SPIRType::Int: break;
			case spirv_cross::SPIRType::UInt: break;
			case spirv_cross::SPIRType::Int64: break;
			case spirv_cross::SPIRType::UInt64: break;
			case spirv_cross::SPIRType::AtomicCounter: break;
			case spirv_cross::SPIRType::Float:
			{
				if (type.vecsize > 0) {
					HandleVectors(shader, type.vecsize);
				}
				if (type.columns > 0) {
					HandleMatrix(shader, type);
				}
				break;
			}
			case spirv_cross::SPIRType::Double: break;
			case spirv_cross::SPIRType::Struct:
			{
				if (type.vecsize > 0) {
					HandleVectors(shader, type.vecsize);
				}
				if (type.columns > 0) {
					HandleMatrix(shader, type);
				}
				break;
			}
			case spirv_cross::SPIRType::Image: break;
			case spirv_cross::SPIRType::SampledImage: break;
			case spirv_cross::SPIRType::Sampler:
			{
				shader.bindings.push_back(Sampler);
				shader.params.push_back(None);
				break;
			}
			case spirv_cross::SPIRType::Unknown: break;
			default:;
			}
		}
	}
	static std::vector<uint32_t> ReadSpirv(const char *path) {
		FILE *file = fopen(path, "rb");
		if (!file) {
			fprintf(stderr, "Failed to open SPIRV file: %s\n", path);
			return {};
		}

		fseek(file, 0, SEEK_END);
		long len = ftell(file) / sizeof(uint32_t);
		rewind(file);

		std::vector<uint32_t> spirv(len);
		if (fread(spirv.data(), sizeof(uint32_t), len, file) != size_t(len))
			spirv.clear();

		fclose(file);
		return spirv;
	}

	void VulkanShader::CreateShaderModule(const std::vector<char>& code, VulkanShader& shader) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned;
		codeAligned.reserve(code.size() / 4 + 1);
		memcpy(codeAligned.data(), code.data(), code.size());

		createInfo.pCode = codeAligned.data();
		const VkResult res = vkCreateShaderModule(context.device, &createInfo, nullptr, &shader.shaderModule);
		VkCheckOrThrow(res, "failed to create shader module!");

		BuildShaderParams(ReadSpirv(shader.shaderid.GetVertexShader().c_str()), shader);
	}
}
