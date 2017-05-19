#pragma once
#include <vulkan/vulkan.h>
#include "VkCom.h"

namespace Vulkan {
	class VulkanRenderer;
	struct VulkanTexture {
		std::string file;
		uint32_t mipMapLevels;
		uint32_t layerCount;
		uint32_t textureWidth;
		uint32_t textureHeight;

		VkCom<VkImage> textureImage;
		VkCom<VkDeviceMemory> textureImageMemory;
		VkCom<VkImageView> textureImageView;
		VkCom<VkSampler> textureSampler;
		VkDescriptorImageInfo imageInfo;

		VulkanRenderer* pRenderer;

		VulkanTexture();
		explicit VulkanTexture(VulkanRenderer* renderer);
		void SetRenderDevice(VulkanRenderer* renderer);
		void Generate();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
	};
}
