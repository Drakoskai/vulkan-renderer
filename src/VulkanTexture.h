#ifndef VULKAN_TEXTURE_H_
#define VULKAN_TEXTURE_H_

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "VkCom.h"
#include "VulkanTypes.h"

namespace Vulkan {
	class VulkanTexture {
	public:
		static VulkanTexture* GetTexture(TextureId textureid);

		VulkanTexture();
		~VulkanTexture();
		void Generate();
		const VkCom<VkImageView>& GetImageView() const;
		const VkCom<VkSampler>& GetSampler() const;
		const VkDescriptorImageInfo& GetImageInfo() const;
	private:
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();

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
	
		static std::unordered_map<TextureId, VulkanTexture> textures_;		
	};
}
#endif
