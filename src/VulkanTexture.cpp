#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Vulkan
{
	VulkanTexture::VulkanTexture() : mipMapLevels(0), layerCount(0), textureWidth(0), textureHeight(0), pRenderer(nullptr) {}

	VulkanTexture::VulkanTexture(VulkanRenderer* renderer) : mipMapLevels(0), layerCount(0), textureWidth(0), textureHeight(0), pRenderer(renderer) {
		textureImage = { pRenderer->device, vkDestroyImage };
		textureImageMemory = { pRenderer->device, vkFreeMemory };
		textureImageView = { pRenderer->device, vkDestroyImageView };
		textureSampler = { pRenderer->device, vkDestroySampler };
	}

	void VulkanTexture::SetRenderDevice(VulkanRenderer* renderer) {
		pRenderer = renderer;
		textureImage = { pRenderer->device, vkDestroyImage };
		textureImageMemory = { pRenderer->device, vkFreeMemory };
		textureImageView = { pRenderer->device, vkDestroyImageView };
		textureSampler = { pRenderer->device, vkDestroySampler };
	}

	void VulkanTexture::Generate() {
		CreateTextureImage();
		CreateTextureImageView();
		CreateTextureSampler();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;
	}

	void VulkanTexture::CreateTextureImage() {
		int texWidth;
		int texHeight;
		int texChannels;
		stbi_uc* pixels = stbi_load(file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		textureWidth = texWidth;
		textureHeight = textureHeight;
		VkCom<VkImage> stagingImage{ pRenderer->device, vkDestroyImage };
		VkCom<VkDeviceMemory> stagingImageMemory{ pRenderer->device, vkFreeMemory };
		pRenderer->CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);

		VkImageSubresource subresource = {};
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.arrayLayer = 0;

		VkSubresourceLayout stagingImageLayout;
		vkGetImageSubresourceLayout(pRenderer->device, stagingImage, &subresource, &stagingImageLayout);

		void* data;
		vkMapMemory(pRenderer->device, stagingImageMemory, 0, imageSize, 0, &data);

		if (stagingImageLayout.rowPitch == texWidth * 4) {
			memcpy(data, pixels, static_cast<size_t>(imageSize));
		}
		else {
			uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

			for (int y = 0; y < texHeight; y++) {
				memcpy(&dataBytes[y * stagingImageLayout.rowPitch], &pixels[y * texWidth * 4], texWidth * 4);
			}
		}

		vkUnmapMemory(pRenderer->device, stagingImageMemory);

		stbi_image_free(pixels);

		pRenderer->CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		pRenderer->TransitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		pRenderer->TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		pRenderer->CopyImage(stagingImage, textureImage, texWidth, texHeight);

		pRenderer->TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanTexture::CreateTextureImageView() {
		pRenderer->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView);
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
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(pRenderer->device, &samplerInfo, nullptr, textureSampler.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}
