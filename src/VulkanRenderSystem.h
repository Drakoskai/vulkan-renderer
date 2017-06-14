#ifndef VULKAN_RENDERER_H_
#define VULKAN_RENDERER_H_

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "GfxTypes.h"
#include "VkCom.h"
#include "Util.h"
#include "VulkanDrawable.h"

namespace Vulkan {

	const int WIDTH = 800;
	const int HEIGHT = 600;
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	inline VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
		auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	inline void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
		auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
		if (func != nullptr) {
			func(instance, callback, pAllocator);
		}
	}
	class VulkanRenderSystem : public IRenderer {
	public:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* userData) {
			std::string prefix("");
			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
				prefix += "ERROR:";
			}
			if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
				prefix += "WARNING:";
			}
			if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
				prefix += "PERFORMANCE:";
			}
			if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
				prefix += "INFO:";
			}
			if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
				prefix += "DEBUG:";
			}

			DebugPrintF("%s [%s] Code %i : %s\n", prefix.c_str(), pLayerPrefix, msgCode, pMsg);

			return VK_FALSE;
		}

		static void onWindowResized(GLFWwindow* window, int width, int height) {
			if (width == 0 || height == 0) return;

			VulkanRenderSystem* renderer = reinterpret_cast<VulkanRenderSystem*>(glfwGetWindowUserPointer(window));
			renderer->RecreateSwapChain();
		}

		VulkanRenderSystem(GLFWwindow* window);
		~VulkanRenderSystem();

		void Init() override;
		void PrepareFrame() override;
		void PresentFrame() override;
		void EndFrame() override;
		static VulkanDrawable* GetDrawable();
		
		static VkCommandBuffer BeginCommandBuffer();
		static void EndCommandBuffer(VkCommandBuffer commandBuffer);

		void RecreateSwapChain();
		static void CreateInstance();
		void SetupDebugCallback();
		void CreateSurface() const;
		static void PickPhysicalDevice();
		static void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		static void CreateRenderPass();
		static void CreateCommandPool();
		void CreateDepthResources();
		void CreateFramebuffers();
		void CreateCommandBuffers();
		void CreateSemaphores();
		static void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkCom<VkImageView>& imageView);
		static void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkCom<VkImage>& image, VkCom<VkDeviceMemory>& imageMemory);
		static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkCom<VkBuffer>& buffer, VkCom<VkDeviceMemory>& bufferMemory);
		static void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
		static void CopyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);
		static void CreateShaderModule(const std::vector<char>& code, VkCom<VkShaderModule>& shaderModule);
		
		static GLFWwindow* pGLFWwindow;
		static VkCom<VkInstance> Instance;
		static VkCom<VkSurfaceKHR> Surface;
		static VkPhysicalDevice PhysicalDevice;
		static VkCom<VkDevice> Device;
		static VkFormat SwapChainImageFormat;
		static VkExtent2D SwapChainExtent;
		static VkCom<VkSwapchainKHR> SwapChain;

		static VkQueue GraphicsQueue;
		static VkQueue PresentQueue;
		static VkCom<VkCommandPool> PrimaryCommandPool;
		static std::vector<VkCommandBuffer> PrimaryCommandBuffer;
		static VkCom<VkRenderPass> RenderPass;
		static std::vector<VulkanDrawable> Drawables;
		
	private:
		static uint32_t currentDrawable_;
		VkCom<VkDebugReportCallbackEXT> callback_;
		VkCom<VkImage> depthImage_;
		VkCom<VkDeviceMemory> depthImageMemory_;
		VkCom<VkImageView> depthImageView_;
		VkCom<VkSemaphore> imageAvailableSemaphore_;
		VkCom<VkSemaphore> renderFinishedSemaphore_;
		std::vector<VkImage> swapChainImages_;
		std::vector<VkCom<VkImageView>> swapChainImageViews_;
			
		std::vector<VkCom<VkFramebuffer>> swapChainFramebuffers_;
	};
}
#endif
