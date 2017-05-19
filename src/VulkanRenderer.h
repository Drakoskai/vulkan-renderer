#ifndef VULKAN_RENDERER_H_
#define VULKAN_RENDERER_H_

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <unordered_map>
#include <GLFW/glfw3.h>
#include "GraphicsStructs.h"
#include "VkCom.h"
#include "VulkanTypes.h"
#include "Vertex.h"
#include "Util.h"
#include "Mesh.h"
#include "VulkanTexture.h"

namespace Vulkan {

	const int WIDTH = 800;
	const int HEIGHT = 600;
	const std::string CUBE_TEXTURE_PATH = "models/cube/texture/cube_dm.png";
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

	class VulkanRenderer : public IRenderer {
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

			VulkanRenderer* renderer = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
			renderer->RecreateSwapChain();
		}

		VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer();

		void Init() override;
		void PrepareFrame() override;
		void DrawFrame() override;
		void EndFrame() override;
		VulkanDrawable* GetDrawable();
		VulkanShader* GetShader(ShaderId shaderid);
		VulkanTexture* GetTexture(TextureId textureid);
		VulkanPipeline* GetPipeline(uint64_t pipelinehash);
		VkCommandBuffer BeginCommandBuffer() const;
		void EndCommandBuffer(VkCommandBuffer commandBuffer) const;

		void RecreateSwapChain();
		void CreateInstance();
		void SetupDebugCallback();
		void CreateSurface();
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

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		VkFormat FindDepthFormat() const;
		static bool HasStencilComponent(VkFormat format);
		void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkCom<VkImageView>& imageView) const;
		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkCom<VkImage>& image, VkCom<VkDeviceMemory>& imageMemory) const;
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height) const;
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkCom<VkBuffer>& buffer, VkCom<VkDeviceMemory>& bufferMemory) const;
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		void CreateShaderModule(const std::vector<char>& code, VkCom<VkShaderModule>& shaderModule) const;
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
		bool IsDeviceSuitable(VkPhysicalDevice device) const;
		static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
		static std::vector<const char*> GetRequiredExtensions();
		static bool CheckValidationLayerSupport();

		void InitMesh(Mesh* mesh) override { mesh->SetRenderDevice(this); }
		GLFWwindow* pWnd_;

		VkCom<VkInstance> instance_{ vkDestroyInstance };
		VkCom<VkDebugReportCallbackEXT> callback_{ instance_, DestroyDebugReportCallbackEXT };
		VkCom<VkSurfaceKHR> surface{ instance_, vkDestroySurfaceKHR };

		VkPhysicalDevice physicalDevice_ = nullptr;
		VkCom<VkDevice> device_{ vkDestroyDevice };

		VkQueue graphicsQueue_;
		VkQueue presentQueue_;

		VkCom<VkSwapchainKHR> swapChain_{ device_, vkDestroySwapchainKHR };
		std::vector<VkImage> swapChainImages_;
		VkFormat swapChainImageFormat_;
		VkExtent2D swapChainExtent_;
		VkCom<VkCommandPool> cmdPool_{ device_, vkDestroyCommandPool };
		VkCom<VkRenderPass> renderPass_{ device_, vkDestroyRenderPass };
		//framebuffer
		VkCom<VkImage> depthImage_{ device_, vkDestroyImage };
		VkCom<VkDeviceMemory> depthImageMemory_{ device_, vkFreeMemory };
		VkCom<VkImageView> depthImageView_{ device_, vkDestroyImageView };
		VkCom<VkSemaphore> imageAvailableSemaphore_{ device_, vkDestroySemaphore };
		VkCom<VkSemaphore> renderFinishedSemaphore_{ device_, vkDestroySemaphore };

		uint32_t currentDrawable_ = 0;
		std::vector<VulkanDrawable> drawables_;
		std::vector<VkCommandBuffer> cmdBuffers_;
		std::vector<VkCom<VkImageView>> swapChainImageViews_;
		std::vector<VkCom<VkFramebuffer>> swapChainFramebuffers_;
		std::unordered_map<ShaderId, VulkanShader> shadercache_;
		std::unordered_map<TextureId, VulkanTexture> textures_;
		std::unordered_map<uint64_t, VulkanPipeline> pipelines_;

	};
}
#endif
