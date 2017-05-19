#ifndef VULKAN_TEXTURE_H_
#define VULKAN_TEXTURE_H_

#include <vulkan/vulkan.h>
#include "VkCom.h"

namespace Vulkan {
	class VulkanRenderer;
	struct VulkanTexture {
		std::string file_;
		uint32_t mipMapLevels_;
		uint32_t layerCount_;
		uint32_t textureWidth_;
		uint32_t textureHeight_;

		VkCom<VkImage> textureImage_;
		VkCom<VkDeviceMemory> textureImageMemory_;
		VkCom<VkImageView> textureImageView_;
		VkCom<VkSampler> textureSampler_;
		VkDescriptorImageInfo imageInfo_;

		VulkanRenderer* pRenderer_;

		VulkanTexture();
		explicit VulkanTexture(VulkanRenderer* renderer);
		void SetRenderDevice(VulkanRenderer* renderer);
		void Generate();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
	};
}
#endif
