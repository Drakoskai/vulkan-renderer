#ifndef VULKAN_TEXTURE_H_
#define VULKAN_TEXTURE_H_

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "VulkanTypes.h"

namespace Vulkan {
	class VulkanTexture {
	public:
		static VulkanTexture* GetTexture(TextureId textureid);

		VulkanTexture();
		~VulkanTexture();
		void Generate();
		VkImageView GetImageView() const;
		VkSampler GetSampler() const;
		VkDescriptorImageInfo GetImageInfo() const;
	private:
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();

		std::string mFile;
		uint32_t mMipMapLevels;
		uint32_t mLayerCount;
		uint32_t mTextureWidth;
		uint32_t mTextureHeight;

		VkImage hTextureImage = VK_NULL_HANDLE;
		VkDeviceMemory hTextureImageMemory = VK_NULL_HANDLE;
		VkImageView hTextureImageView = VK_NULL_HANDLE;
		VkSampler hTextureSampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo mImageInfo;
	
		static std::unordered_map<TextureId, VulkanTexture> Textures;		
	};
}
#endif
