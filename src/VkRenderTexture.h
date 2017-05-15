#pragma once

#include <vulkan/vulkan.h>
#include "VkCom.h"
#include "Texture.h"

class RenderContext;

class VkRenderTexture : public AbstractTexture {
public:
	VkRenderTexture();
	~VkRenderTexture();

	const VkCom<VkFramebuffer>& GetFrameBuffer() const { return hFrameBuffer; }
	const VkCom<VkImageView>& GetColorView() const { return mColorAttachment.hImageView; }
	const VkCom<VkRenderPass>& GetRenderPass() const { return hRenderPass; }
	void Create(const RenderContext& ctx, VkFormat swapChainImageFormat, VkFormat depthFormat);

private:
	void CreateRenderPass(const RenderContext& ctx, VkFormat swapChainImageFormat, VkFormat depthFormat);

	struct FrameBufferAttachment {
		VkCom<VkImage> hImage{ VK_NULL_HANDLE };
		VkCom<VkImageView> hImageView{ VK_NULL_HANDLE };
		VkCom<VkDeviceMemory> hImageMemmory{ VK_NULL_HANDLE };
	};

	VkCom<VkFramebuffer> hFrameBuffer{ VK_NULL_HANDLE };
	FrameBufferAttachment mColorAttachment{ VK_NULL_HANDLE };
	FrameBufferAttachment mDepthAttachment{ VK_NULL_HANDLE };
	VkCom<VkImage> hImage{ VK_NULL_HANDLE };
	VkCom<VkImageLayout> hImageLayout{ VK_NULL_HANDLE };
	VkCom<VkDeviceMemory> hDeviceMemmory{ VK_NULL_HANDLE };
	VkCom<VkImageView> hImageView{ VK_NULL_HANDLE };
	VkCom<VkRenderPass> hRenderPass{ VK_NULL_HANDLE };

};
