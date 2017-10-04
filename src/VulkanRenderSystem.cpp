#include "VulkanRenderSystem.h"
#include <set>
#include <chrono>
#include "VulkanTypes.h"
#include "VulkanUtil.h"
#include "VulkanInit.h"
#include "VulkanBufferStaging.h"
#include "VulkanAllocator.h"
#include "Camera.h"
#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Vulkan {

	VkInstance VulkanRenderSystem::Instance = VK_NULL_HANDLE;
	VkSurfaceKHR VulkanRenderSystem::Surface = VK_NULL_HANDLE;
	VkFormat VulkanRenderSystem::SwapChainImageFormat;
	VkExtent2D VulkanRenderSystem::SwapChainExtent;
	VkSwapchainKHR VulkanRenderSystem::SwapChain = VK_NULL_HANDLE;
	GLFWwindow* VulkanRenderSystem::pGLFWwindow = nullptr;
	VulkanContext context;
	VulkanAllocator Allocator;

	VulkanRenderSystem::VulkanRenderSystem(GLFWwindow* window) : mCurrentSwapImage(0), mPipelineState(0) {
		pGLFWwindow = window;
	}

	VulkanRenderSystem::~VulkanRenderSystem() { Shutdown(); }

	void VulkanRenderSystem::Init() {
		mPipelineState = GetDefaultPipelineState();
		CreateInstance();
		SetupDebugCallback();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		Allocator.Init();
		Staging.Init();
		GpuProgramState.Init();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateCommandPool();
		CreateDepthResources();
		CreateFramebuffers();
		CreateCommandBuffers();
		CreateSemaphores();
	}

	void VulkanRenderSystem::Shutdown() {
		vkDeviceWaitIdle(context.device);
		CleanupSwapChain();

		for (auto i = 0; i < NumberOfFrameBuffers; i++) {
			vkDestroySemaphore(context.device, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(context.device, mRenderFinishedSemaphore[i], nullptr);
		}
		
		GpuProgramState.Shutdown();
		Staging.Shutdown();
		Allocator.Shutdown();
		vkDestroyDevice(context.device, nullptr);
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
		DestroyDebugReportCallback(Instance);
		vkDestroyInstance(Instance, nullptr);
	}

	void VulkanRenderSystem::CleanupSwapChain() {
		vkDestroyImageView(context.device, depthImageView_, nullptr);
		vkDestroyImage(context.device, depthImage_, nullptr);
		vkFreeMemory(context.device, depthImageMemory_, nullptr);

		for (auto i = 0; i < swapChainFramebuffers_.size(); i++) {
			vkDestroyFramebuffer(context.device, swapChainFramebuffers_[i], nullptr);
		}

		vkFreeCommandBuffers(context.device, context.commandPool, static_cast<uint32_t>(context.commandBuffers.size()), context.commandBuffers.data());
		vkDestroyRenderPass(context.device, context.renderPass, nullptr);

		for (auto i = 0; i < swapChainImageViews_.size(); i++) {
			vkDestroyImageView(context.device, swapChainImageViews_[i], nullptr);
		}

		vkDestroySwapchainKHR(context.device, SwapChain, nullptr);
	}

	void VulkanRenderSystem::PrepareFrame() {
		Transform::UpdateLocalMatrices();
		Camera::GetCameras()[0].SetProjection(glm::perspective(glm::radians(45.0f), SwapChainExtent.width / static_cast<float>(SwapChainExtent.height), 0.1f, 10.0f));

		UniformBufferObject ubo;
		Matrix model = Transform::GetTransforms()[0].GetModelToWorldMatrix();
		Matrix view = Camera::GetCameras()[0].GetView();
		Matrix proj = Camera::GetCameras()[0].GetProj();
		proj[1][1] *= -1;

		ubo.model = model;
		ubo.view = view;
		ubo.proj = proj;
		ubo.world = Matrix(1.0);
		ubo.lightpos = Vec3(2.0f, 2.0f, 8.0f);
		std::array<UniformBufferObject, 1> uniforms = { ubo };

		GpuProgramState.SetUniformData(context.currentFrame, uniforms.data());		
	}

	void VulkanRenderSystem::StartFrame() {
		const uint32_t currentFrame = context.currentFrame;
		context.commandBuffer = context.commandBuffers[currentFrame];
		const VkResult result = vkAcquireNextImageKHR(context.device, SwapChain, UINT64_MAX, mImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &mCurrentSwapImage);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		Allocator.EmptyGarbage();
		Staging.Flush();
		GpuProgramState.Start();
	}

	void VulkanRenderSystem::SwapBuffers() {
		const uint32_t currentFrame = context.currentFrame;
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &context.commandBuffer;

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore[context.currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &mCurrentSwapImage;

		VkResult result = vkQueuePresentKHR(context.presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		vkQueueWaitIdle(context.presentQueue);
		context.counter++;
		context.currentFrame = context.counter % NumberOfFrameBuffers;
	}

	void VulkanRenderSystem::DrawFrame() {
		const VulkanRenderProgram& renderProgram = GpuProgramState.GetCurrentRenderProg();
		GpuProgramState.Commit(mPipelineState);
	}

	void VulkanRenderSystem::PresentFrame() {
		DrawFrame();
		SwapBuffers();
	}

	void VulkanRenderSystem::EndFrame() {}

	void VulkanRenderSystem::EnumeratePhysicalDevices() {
		uint32_t deviceCount = 0;
		VkCheck(vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr));
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);

		VkCheck(vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data()));
		DebugPrintFIf(deviceCount > 0, "vkEnumeratePhysicalDevices returned zero devices.");

		context.gpus.resize(deviceCount);

		for (uint32_t i = 0; i < deviceCount; ++i) {
			GpuInfo & gpu = context.gpus[i];
			gpu.physicalDevice = devices[i];
			{
				uint32_t numQueues = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(gpu.physicalDevice, &numQueues, NULL);
				DebugPrintFIf(numQueues > 0, "vkGetPhysicalDeviceQueueFamilyProperties returned zero queues.");

				gpu.queueFamilies.resize(numQueues);
				vkGetPhysicalDeviceQueueFamilyProperties(gpu.physicalDevice, &numQueues, gpu.queueFamilies.data());
				DebugPrintFIf(numQueues > 0, "vkGetPhysicalDeviceQueueFamilyProperties returned zero queues.");
			}
			{
				uint32_t numExtension;
				VkCheck(vkEnumerateDeviceExtensionProperties(gpu.physicalDevice, NULL, &numExtension, NULL));
				DebugPrintFIf(numExtension > 0, "vkEnumerateDeviceExtensionProperties returned zero extensions.");

				gpu.availableExtensions.resize(numExtension);
				VkCheck(vkEnumerateDeviceExtensionProperties(gpu.physicalDevice, NULL, &numExtension, gpu.availableExtensions.data()));
				DebugPrintFIf(numExtension > 0, "vkEnumerateDeviceExtensionProperties returned zero extensions.");
			}

			VkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.physicalDevice, Surface, &gpu.surfaceCaps));
			{
				uint32_t numFormats;
				VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physicalDevice, Surface, &numFormats, NULL));
				DebugPrintFIf(numFormats > 0, "vkGetPhysicalDeviceSurfaceFormatsKHR returned zero surface formats.");

				gpu.surfaceFormats.resize(numFormats);
				VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.physicalDevice, Surface, &numFormats, gpu.surfaceFormats.data()));
				DebugPrintFIf(numFormats > 0, "vkGetPhysicalDeviceSurfaceFormatsKHR returned zero surface formats.");
			}
			{
				uint32_t numPresentModes;
				VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.physicalDevice, Surface, &numPresentModes, NULL));
				DebugPrintFIf(numPresentModes > 0, "vkGetPhysicalDeviceSurfacePresentModesKHR returned zero present modes.");

				gpu.presentModes.resize(numPresentModes);
				VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.physicalDevice, Surface, &numPresentModes, gpu.presentModes.data()));
				DebugPrintFIf(numPresentModes > 0, "vkGetPhysicalDeviceSurfacePresentModesKHR returned zero present modes.");
			}

			vkGetPhysicalDeviceMemoryProperties(gpu.physicalDevice, &gpu.memProperties);
			vkGetPhysicalDeviceProperties(gpu.physicalDevice, &gpu.physicalDeviceProps);
		}
	}

	void VulkanRenderSystem::RecreateSwapChain() {
		vkDeviceWaitIdle(context.device);
		CleanupSwapChain();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthResources();
		CreateFramebuffers();
		CreateCommandBuffers();
	}

	void VulkanRenderSystem::CreateInstance() {
		if (enableValidationLayers) {
			CheckValidationLayerSupport();
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Engine Test";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Kai Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION);;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}
		VkCheckOrThrow(vkCreateInstance(&createInfo, nullptr, &Instance), "failed to create instance!");
	}

	void VulkanRenderSystem::SetupDebugCallback() {
		if (enableValidationLayers) {
			CreateDebugReportCallback(Instance);
		}
	}

	void VulkanRenderSystem::CreateSurface() {
		VkCheckOrThrow(glfwCreateWindowSurface(Instance, pGLFWwindow, nullptr, &Surface), "failed to create window surface!");
	}

	void VulkanRenderSystem::PickPhysicalDevice() {
		EnumeratePhysicalDevices();
		for (int i = 0; i < context.gpus.size(); ++i) {
			GpuInfo & gpu = context.gpus[i];

			int graphicsIdx = -1;
			int presentIdx = -1;

			if (!CheckDeviceExtensionSupport(gpu, deviceExtensions)) {
				continue;
			}

			if (gpu.surfaceFormats.size() == 0) {
				continue;
			}

			if (gpu.presentModes.size() == 0) {
				continue;
			}

			for (int j = 0; j < gpu.queueFamilies.size(); ++j) {
				VkQueueFamilyProperties & props = gpu.queueFamilies[j];

				if (props.queueCount == 0) {
					continue;
				}

				if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					graphicsIdx = j;
					break;
				}
			}

			for (int j = 0; j < gpu.queueFamilies.size(); ++j) {
				VkQueueFamilyProperties & props = gpu.queueFamilies[j];

				if (props.queueCount == 0) {
					continue;
				}

				VkBool32 supportsPresent = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu.physicalDevice, j, Surface, &supportsPresent);
				vkGetPhysicalDeviceFeatures(gpu.physicalDevice, &gpu.physicalDeviceFeatures);
				if (supportsPresent) {
					presentIdx = j;
					break;
				}
			}

			if (graphicsIdx >= 0 && presentIdx >= 0) {
				context.graphicsFamilyIdx = graphicsIdx;
				context.presentFamilyIdx = presentIdx;
				context.gpu = &gpu;
				return;
			}
		}
		if (context.gpu->physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void VulkanRenderSystem::CreateLogicalDevice() {
		std::array<int, 2> uniqueQueueFamilies{
			context.graphicsFamilyIdx,
			context.presentFamilyIdx
		};

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.textureCompressionBC = VK_TRUE;
		deviceFeatures.imageCubeArray = VK_TRUE;
		deviceFeatures.depthClamp = VK_TRUE;
		deviceFeatures.depthBiasClamp = VK_TRUE;
		deviceFeatures.depthBounds = VK_TRUE;
		deviceFeatures.fillModeNonSolid = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		VkCheckOrThrow(vkCreateDevice(context.gpu->physicalDevice, &createInfo, nullptr, &context.device), "Failed to create Logical Device!");

		vkGetDeviceQueue(context.device, context.graphicsFamilyIdx, 0, &context.graphicsQueue);
		vkGetDeviceQueue(context.device, context.presentFamilyIdx, 0, &context.presentQueue);
	}

	void VulkanRenderSystem::CreateSwapChain() {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = Surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(context.graphicsFamilyIdx), static_cast<uint32_t>(context.presentFamilyIdx) };
		if (context.graphicsFamilyIdx != context.presentFamilyIdx) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		VkSwapchainKHR oldSwapChain = SwapChain;
		createInfo.oldSwapchain = oldSwapChain;

		VkSwapchainKHR newSwapChain;
		VkCheckOrThrow(vkCreateSwapchainKHR(context.device, &createInfo, nullptr, &newSwapChain), "failed to create swap chain!");

		SwapChain = newSwapChain;

		vkGetSwapchainImagesKHR(context.device, SwapChain, &imageCount, nullptr);
		swapChainImages_.resize(imageCount);
		vkGetSwapchainImagesKHR(context.device, SwapChain, &imageCount, swapChainImages_.data());

		SwapChainImageFormat = surfaceFormat.format;
		SwapChainExtent = extent;
	}

	void VulkanRenderSystem::CreateImageViews() {
		swapChainImageViews_.resize(swapChainImages_.size(), VK_NULL_HANDLE);

		for (uint32_t i = 0; i < swapChainImages_.size(); i++) {
			CreateImageView(swapChainImages_[i], SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews_[i]);
		}
	}

	void VulkanRenderSystem::CreateRenderPass() {
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = SwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef;
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkCheckOrThrow(vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &context.renderPass), "failed to create render pass!");
	}

	void VulkanRenderSystem::CreateCommandPool() {
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = context.graphicsFamilyIdx;
		const VkResult res = vkCreateCommandPool(context.device, &poolInfo, nullptr, &context.commandPool);
		VkCheckOrThrow(res, "failed to create graphics command pool!");
	}

	void VulkanRenderSystem::CreateDepthResources() {
		const VkFormat depthFormat = FindDepthFormat();
		CreateImage(SwapChainExtent.width, SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage_, depthImageMemory_);
		CreateImageView(depthImage_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView_);
		TransitionImageLayout(depthImage_, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void VulkanRenderSystem::CreateFramebuffers() {
		swapChainFramebuffers_.resize(swapChainImageViews_.size(), VK_NULL_HANDLE);
		for (int i = 0; i < swapChainImageViews_.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				swapChainImageViews_[i],
				depthImageView_
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = context.renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = SwapChainExtent.width;
			framebufferInfo.height = SwapChainExtent.height;
			framebufferInfo.layers = 1;

			const VkResult res = vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]);
			VkCheckOrThrow(res, "failed to create framebuffer!");
		}
	}

	void VulkanRenderSystem::CreateCommandBuffers() {
		if (context.commandBuffers.size() > 0) {
			vkFreeCommandBuffers(context.device, context.commandPool, static_cast<uint32_t>(context.commandBuffers.size()), context.commandBuffers.data());
		}

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = context.commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(context.commandBuffers.size());
		const VkResult res = vkAllocateCommandBuffers(context.device, &allocInfo, context.commandBuffers.data());
		VkCheckOrThrow(res, "failed to allocate command buffers!");

		for (auto i = 0; i < context.commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			vkBeginCommandBuffer(context.commandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = context.renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers_[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = SwapChainExtent;

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(context.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = {
			0.0f,
			0.0f,
			static_cast<float>(SwapChainExtent.width),
			static_cast<float>(SwapChainExtent.height),
			0.0f,
			1.0f };

			vkCmdSetViewport(context.commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = {
			{ 0, 0 },
			SwapChainExtent };

			vkCmdSetScissor(context.commandBuffers[i], 0, 1, &scissor);
			vkCmdEndRenderPass(context.commandBuffers[i]);
			vkEndCommandBuffer(context.commandBuffers[i]);
		}
	}

	void CreateSemaphore(VkSemaphore& semaphore) {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkCheckOrThrow(vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &semaphore), "failed to create semaphore!");
	}

	void VulkanRenderSystem::CreateSemaphores() {
		for (auto i = 0; i < NumberOfFrameBuffers; i++) {
			VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
			CreateSemaphore(imageAvailableSemaphore);
			mImageAvailableSemaphores[i] = imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
			CreateSemaphore(renderFinishedSemaphore);
			mRenderFinishedSemaphore[i] = renderFinishedSemaphore;
		}
	}

	SwapChainSupportDetails VulkanRenderSystem::QuerySwapChainSupport() {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.gpu->physicalDevice, Surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(context.gpu->physicalDevice, Surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(context.gpu->physicalDevice, Surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(context.gpu->physicalDevice, Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(context.gpu->physicalDevice, Surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkExtent2D VulkanRenderSystem::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			int width, height;
			glfwGetWindowSize(pGLFWwindow, &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
}
