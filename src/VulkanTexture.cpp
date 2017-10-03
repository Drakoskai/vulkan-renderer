#include "VulkanTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VulkanInit.h"
#include "Util.h"

namespace Vulkan {
	std::unordered_map<TextureId, VulkanTexture> VulkanTexture::Textures;

	VulkanTexture::VulkanTexture() : mMipMapLevels(0), mLayerCount(0), mTextureWidth(0), mTextureHeight(0) {}

	VulkanTexture::~VulkanTexture() {
		vkDestroySampler(context.device, hTextureSampler, nullptr);
		vkDestroyImageView(context.device, hTextureImageView, nullptr);
		vkFreeMemory(context.device, hTextureImageMemory, nullptr);
		vkDestroyImage(context.device, hTextureImage, nullptr);
	}

	void VulkanTexture::Generate() {
		CreateTextureImage();
		CreateTextureImageView();
		CreateTextureSampler();
		mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		mImageInfo.imageView = hTextureImageView;
		mImageInfo.sampler = hTextureSampler;
	}

	VkImageView VulkanTexture::GetImageView() const {
		return hTextureImageView;
	}

	VkSampler VulkanTexture::GetSampler() const {
		return hTextureSampler;
	}

	VkDescriptorImageInfo VulkanTexture::GetImageInfo() const {
		return mImageInfo;
	}

	void VulkanTexture::CreateTextureImage() {
		int texWidth;
		int texHeight;
		int texChannels;
		std::string filepath(TEXTURES_PATH + mFile);
		stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image! " + filepath);
		}

		mTextureWidth = texWidth;
		mTextureHeight = mTextureHeight;
		VkImage stagingImage = VK_NULL_HANDLE;
		VkDeviceMemory stagingImageMemory = VK_NULL_HANDLE;
		CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);

		VkImageSubresource subresource;
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.arrayLayer = 0;

		VkSubresourceLayout stagingImageLayout;
		vkGetImageSubresourceLayout(context.device, stagingImage, &subresource, &stagingImageLayout);

		void* data;
		vkMapMemory(context.device, stagingImageMemory, 0, imageSize, 0, &data);

		if (stagingImageLayout.rowPitch == texWidth * 4) {
			memcpy(data, pixels, imageSize);
		} else {
			uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

			for (int y = 0; y < texHeight; y++) {
				memcpy(&dataBytes[y * stagingImageLayout.rowPitch], &pixels[y * texWidth * 4], texWidth * 4);
			}
		}

		vkUnmapMemory(context.device, stagingImageMemory);

		stbi_image_free(pixels);

		CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, hTextureImage, hTextureImageMemory);
		TransitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		TransitionImageLayout(hTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyImage(stagingImage, hTextureImage, texWidth, texHeight);
		TransitionImageLayout(hTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkFreeMemory(context.device, stagingImageMemory, nullptr);
		vkDestroyImage(context.device, stagingImage, nullptr);
	}

	void VulkanTexture::CreateTextureImageView() {
		CreateImageView(hTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, hTextureImageView);
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

		if (vkCreateSampler(context.device, &samplerInfo, nullptr, &hTextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	VulkanTexture* VulkanTexture::GetTexture(TextureId textureid) {
		if (Textures.find(textureid) == end(Textures)) {
			VulkanTexture t;
			Textures[textureid] = t;
			Textures[textureid].mFile = textureid.GetTextureName();
			Textures[textureid].Generate();
		}
		return &Textures[textureid];
	}
}
