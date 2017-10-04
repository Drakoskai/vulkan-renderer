#ifndef VULKAN_RENDERER_H_
#define VULKAN_RENDERER_H_

#include "stdafx.h"
#include "VulkanRenderProgram.h"
#include "VulkanTypes.h"
#include "GfxTypes.h"

namespace Vulkan {

	const int WIDTH = 800;
	const int HEIGHT = 600;

	class VulkanRenderSystem : public IRenderer {
	public:
		static void onWindowResized(GLFWwindow* window, int width, int height) {
			if (width == 0 || height == 0) return;

			VulkanRenderSystem* renderer = reinterpret_cast<VulkanRenderSystem*>(glfwGetWindowUserPointer(window));
			renderer->RecreateSwapChain();
		}

		VulkanRenderSystem(GLFWwindow* window);
		~VulkanRenderSystem();

		void Init() override;
		void CleanupSwapChain();
		void Shutdown();
		void PrepareFrame() override;
		void StartFrame();
		void SwapBuffers();
		void DrawFrame();
		void PresentFrame() override;
		void EndFrame() override;
		void RecreateSwapChain();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateCommandPool();
		void CreateDepthResources();
		void CreateFramebuffers();
		void CreateCommandBuffers();
		void CreateSemaphores();
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		static void SetupDebugCallback();
		static void CreateSurface();
		static void EnumeratePhysicalDevices();
		static SwapChainSupportDetails QuerySwapChainSupport();

		static GLFWwindow* pGLFWwindow;
		static VkInstance Instance;
		static VkSurfaceKHR Surface;		
		static VkFormat SwapChainImageFormat;
		static VkExtent2D SwapChainExtent;
		static VkSwapchainKHR SwapChain;

	private:		
		VkImage depthImage_ = VK_NULL_HANDLE;
		VkDeviceMemory depthImageMemory_ = VK_NULL_HANDLE;
		VkImageView depthImageView_ = VK_NULL_HANDLE;

		std::vector<VkImage> swapChainImages_;
		std::vector<VkImageView> swapChainImageViews_;
		std::vector<VkFramebuffer> swapChainFramebuffers_;

		std::array<VkSemaphore, NumberOfFrameBuffers> mImageAvailableSemaphores;
		std::array<VkSemaphore, NumberOfFrameBuffers> mRenderFinishedSemaphore;

		std::vector<const char*> instanceExtensions;
		std::vector<const char*> deviceExtensions;
		std::vector<const char*> validationLayers;

		VulkanGpuProgramState GpuProgramState;
		uint32_t mCurrentSwapImage;
		uint64_t mPipelineState;
	};
}
#endif
