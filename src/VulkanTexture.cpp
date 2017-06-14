#include "VulkanTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VulkanRenderSystem.h"

namespace Vulkan {
	std::unordered_map<TextureId, VulkanTexture> VulkanTexture::textures_;

	VulkanTexture::VulkanTexture() : mipMapLevels_(0), layerCount_(0), textureWidth_(0), textureHeight_(0) {}

	VulkanTexture::~VulkanTexture() {}

	void VulkanTexture::Generate() {
		textureImage_ = { VulkanRenderSystem::Device, vkDestroyImage };
		textureImageMemory_ = { VulkanRenderSystem::Device, vkFreeMemory };
		textureImageView_ = { VulkanRenderSystem::Device, vkDestroyImageView };
		textureSampler_ = { VulkanRenderSystem::Device, vkDestroySampler };
		CreateTextureImage();
		CreateTextureImageView();
		CreateTextureSampler();
		imageInfo_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo_.imageView = textureImageView_;
		imageInfo_.sampler = textureSampler_;
	}

	const VkCom<VkImageView>& VulkanTexture::GetImageView() const {
		return textureImageView_;
	}

	const VkCom<VkSampler>& VulkanTexture::GetSampler() const {
		return textureSampler_;
	}

	const VkDescriptorImageInfo& VulkanTexture::GetImageInfo() const {
		return imageInfo_;
	}

	void VulkanTexture::CreateTextureImage() {
		int texWidth;
		int texHeight;
		int texChannels;
		std::string filepath(TEXTURES_PATH + file_);
		stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image! " + filepath);
		}

		textureWidth_ = texWidth;
		textureHeight_ = textureHeight_;
		VkCom<VkImage> stagingImage{ VulkanRenderSystem::Device, vkDestroyImage };
		VkCom<VkDeviceMemory> stagingImageMemory{ VulkanRenderSystem::Device, vkFreeMemory };
		VulkanRenderSystem::CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);

		VkImageSubresource subresource;
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.arrayLayer = 0;

		VkSubresourceLayout stagingImageLayout;
		vkGetImageSubresourceLayout(VulkanRenderSystem::Device, stagingImage, &subresource, &stagingImageLayout);

		void* data;
		vkMapMemory(VulkanRenderSystem::Device, stagingImageMemory, 0, imageSize, 0, &data);

		if (stagingImageLayout.rowPitch == texWidth * 4) {
			memcpy(data, pixels, static_cast<size_t>(imageSize));
		} else {
			uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

			for (int y = 0; y < texHeight; y++) {
				memcpy(&dataBytes[y * stagingImageLayout.rowPitch], &pixels[y * texWidth * 4], texWidth * 4);
			}
		}

		vkUnmapMemory(VulkanRenderSystem::Device, stagingImageMemory);

		stbi_image_free(pixels);

		VulkanRenderSystem::CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage_, textureImageMemory_);
		VulkanRenderSystem::TransitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanRenderSystem::TransitionImageLayout(textureImage_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VulkanRenderSystem::CopyImage(stagingImage, textureImage_, texWidth, texHeight);
		VulkanRenderSystem::TransitionImageLayout(textureImage_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanTexture::CreateTextureImageView() {
		VulkanRenderSystem::CreateImageView(textureImage_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView_);
	}

	void VulkanTexture::CreateTextureSampler() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 1;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(VulkanRenderSystem::Device, &samplerInfo, nullptr, textureSampler_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	VulkanTexture* VulkanTexture::GetTexture(TextureId textureid) {
		if (textures_.find(textureid) == end(textures_)) {
			VulkanTexture t;
			textures_[textureid] = t;
			textures_[textureid].file_ = textureid.GetTextureName();
			textures_[textureid].Generate();
		}
		return &textures_[textureid];
	}
}
