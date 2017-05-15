#pragma once

#include "KaiMath.h"
#include <vulkan/vulkan.h>
#include "GraphicsEnums.h"
#include "VkCom.h"

namespace Vulkan {
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct QueueFamilyIndices {
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
		Matrix mvp;
	};

	struct VkTextureFilter {
		const TextureFilter& mTextureFilter;
		VkTextureFilter(const TextureFilter& textureFilter) : mTextureFilter(textureFilter) { }

		operator VkSamplerMipmapMode() const {
			switch(mTextureFilter) {
			case TextureFilter::Nearest:
				return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			case TextureFilter::Linear:
				return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			default:
				return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			}
		}
	};
}
