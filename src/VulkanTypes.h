#ifndef VULKAN_TYPES_H_
#define VULKAN_TYPES_H_

#include "stdafx.h"
#include "KaiMath.h"
#include "GfxTypes.h"

namespace Vulkan {

	static const int NumberOfFrameBuffers = 2;
	static const int MaxUniformDescriptors = 8192;
	static const int MaxSamplersDescriptors = 12384;
	static const int MaxDesciptorSets = 16384;
	static const int MaxDescriptorSetWrites = 32;
	static const int MaxDescriptorSetUniforms = 48;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const std::vector<const char*> debugExtensions = {
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};

	struct QueueFamilyIndices_t {
		int graphicsFamily = -1;
		int presentFamily = -1;

		bool isComplete() const {
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct UniformBufferObject {
		Matrix model;
		Matrix view;
		Matrix proj;
		Matrix world;
		Vec3 lightpos;
	};

	inline VkCullModeFlagBits GetCullMode(const uint64_t pipelineState) {
		switch (pipelineState & CullMode) {
		case CullModeOff:return VK_CULL_MODE_NONE;
		case CullModeFront:return VK_CULL_MODE_FRONT_BIT;
		case CullModeBack: return VK_CULL_MODE_BACK_BIT;
		default: return VK_CULL_MODE_NONE;
		}
	}

	inline	VkPolygonMode GetPolygonMode(const uint64_t pipelineState) {
		switch (pipelineState & FillMode) {
		case FillModeSolid: return VK_POLYGON_MODE_FILL;
		case FillModeWireWireframe: return VK_POLYGON_MODE_LINE;
		default: return VK_POLYGON_MODE_FILL;
		}
	}

	inline VkBool32 GetBlendMode(const uint64_t pipelineState) {
		switch (pipelineState & BlendMode) {
		case BlendModeOff: return VK_FALSE;
		default: return VK_TRUE;
		}
	}

	inline VkCompareOp GetDepthCompareOp(const uint64_t pipelineState) {
		switch (pipelineState & DepthFunction) {
		case DepthFunctionLessOrEqualWriteOn:
		case DepthFunctionLessOrEqualWriteOff: return VK_COMPARE_OP_LESS_OR_EQUAL;
		case DepthFunctionNoneWriteOff: return VK_COMPARE_OP_NEVER;
		default: return VK_COMPARE_OP_LESS;
		}
	}
}
#endif
