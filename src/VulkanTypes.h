#ifndef VULKAN_TYPES_H__
#define VULKAN_TYPES_H__

#include <vulkan/vulkan.h>
#include "KaiMath.h"
#include "GfxTypes.h"
#include "Material.h"

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
		Matrix model;
		Matrix view;
		Matrix proj;
		Matrix world;
		Vec3 lightpos;
	};

	struct UniformMaterialObject {
		Vec3 ambient;
		Vec3 diffuse;
		Vec3 emissive;
		Vec3 specular;
		float reflection;
		float opacity;
		float shininess;
		int hasDiffuseTexture;
		int hasBumpMapTexture;
		UniformMaterialObject() : reflection(0), opacity(0), shininess(0), hasDiffuseTexture(0), hasBumpMapTexture(0) {}

		UniformMaterialObject(Material material) :
			ambient(material.ambient),
			diffuse(material.diffuse),
			emissive(material.emissive),
			specular(material.specular),
			reflection(material.reflection),
			opacity(material.opacity),
			shininess(material.shininess),
			hasDiffuseTexture(material.diffuseTexture == EmptyTextureId ? 0 : 1),
			hasBumpMapTexture(material.bumpTexture == EmptyTextureId ? 0 : 1) {}
	};

	struct VkTextureFilter {
		const TextureFilter& mTextureFilter;
		VkTextureFilter(const TextureFilter& textureFilter) : mTextureFilter(textureFilter) {}

		operator VkSamplerMipmapMode() const {
			switch (mTextureFilter) {
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
#endif
