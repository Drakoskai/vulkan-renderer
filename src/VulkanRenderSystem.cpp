#include "VulkanRenderSystem.h"
#include <set>
#include <chrono>
#include <glm/gtx/transform.hpp>
#include "VulkanTypes.h"
#include "VulkanUtil.h"
#include "Transform.h"
#include "Camera.h"
#include "Mesh.h"

namespace Vulkan {
	VkCom<VkInstance> VulkanRenderSystem::Instance;
	VkCom<VkSurfaceKHR> VulkanRenderSystem::Surface;
	VkPhysicalDevice VulkanRenderSystem::PhysicalDevice;
	VkCom<VkDevice> VulkanRenderSystem::Device;
	VkFormat VulkanRenderSystem::SwapChainImageFormat;
	VkExtent2D VulkanRenderSystem::SwapChainExtent;
	VkCom<VkSwapchainKHR> VulkanRenderSystem::SwapChain;
	VkCom<VkCommandPool> VulkanRenderSystem::PrimaryCommandPool;
	std::vector<VkCommandBuffer> VulkanRenderSystem::PrimaryCommandBuffer;
	VkQueue VulkanRenderSystem::GraphicsQueue;
	VkQueue VulkanRenderSystem::PresentQueue;
	GLFWwindow* VulkanRenderSystem::pGLFWwindow = nullptr;
	VkCom<VkRenderPass> VulkanRenderSystem::RenderPass;
	std::vector<VulkanDrawable> VulkanRenderSystem::Drawables;
	uint32_t VulkanRenderSystem::currentDrawable_ = 0;

	VulkanRenderSystem::VulkanRenderSystem(GLFWwindow* window) {
		pGLFWwindow = window;
	}

	VulkanRenderSystem::~VulkanRenderSystem() {
		vkDeviceWaitIdle(Device);
		VulkanPipeline::DestroyPipelines();
	}

	void VulkanRenderSystem::Init() {
		Instance = { vkDestroyInstance };
		CreateInstance();
		callback_ = { Instance, DestroyDebugReportCallbackEXT };
		SetupDebugCallback();
		Surface = { Instance, vkDestroySurfaceKHR };
		CreateSurface();
		PhysicalDevice = VK_NULL_HANDLE;
		PickPhysicalDevice();
		Device= { vkDestroyDevice };
		CreateLogicalDevice();
		SwapChain = { Device, vkDestroySwapchainKHR };
		PrimaryCommandPool = { Device, vkDestroyCommandPool };
		depthImage_ = { Device, vkDestroyImage };
		depthImageMemory_ = { Device, vkFreeMemory };
		depthImageView_ = { Device, vkDestroyImageView };
		imageAvailableSemaphore_ = { Device, vkDestroySemaphore };
		renderFinishedSemaphore_ = { Device, vkDestroySemaphore };
		CreateSwapChain();
		CreateImageViews();
		RenderPass = { Device, vkDestroyRenderPass };
		CreateRenderPass();
		CreateCommandPool();
		CreateDepthResources();
		CreateFramebuffers();
		CreateCommandBuffers();
		CreateSemaphores();
	}

	void VulkanRenderSystem::PrepareFrame() {
		RecreateSwapChain();
		Transform::UpdateLocalMatrices();
		Camera::GetCameras()[0].SetProjection(glm::perspective(glm::radians(45.0f), SwapChainExtent.width / static_cast<float>(SwapChainExtent.height), 0.1f, 10.0f));

		UniformBufferObject ubo;
		Matrix model = Transform::GetTransforms()[0].GetModelToWorldMatrix();
		Matrix view = Camera::GetCameras()[0].GetView();
		Matrix proj = Camera::GetCameras()[0].GetProj();
		proj[1][1] *= -1;

		void* data;
		ubo.model = model;
		ubo.view = view;
		ubo.proj = proj;
		ubo.world = Matrix(1.0);
		ubo.lightpos = Vec3(2.0f, 2.0f, 8.0f);

		vkMapMemory(Device, Drawables[0].GetUniformStagingMemory(), 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(Device, Drawables[0].GetUniformStagingMemory());

		CopyBuffer(Drawables[0].GetUniformStagingBuffer(), Drawables[0].GetUniformBuffer(), sizeof(ubo));
	}

	void VulkanRenderSystem::PresentFrame() {
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(Device, SwapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore_, VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore_ };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &PrimaryCommandBuffer[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore_ };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	void VulkanRenderSystem::EndFrame() {}

	VkCommandBuffer VulkanRenderSystem::BeginCommandBuffer() {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = PrimaryCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanRenderSystem::EndCommandBuffer(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue);

		vkFreeCommandBuffers(Device, PrimaryCommandPool, 1, &commandBuffer);
	}

	void VulkanRenderSystem::RecreateSwapChain() {
		vkDeviceWaitIdle(Device);
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthResources();
		CreateFramebuffers();
		CreateCommandBuffers();
	}

	void VulkanRenderSystem::CreateInstance() {
		if (enableValidationLayers && !CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Engine Test";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Kai Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

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

		if (vkCreateInstance(&createInfo, nullptr, Instance.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void VulkanRenderSystem::SetupDebugCallback() {
		if (!enableValidationLayers) return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = DebugCallback;

		if (CreateDebugReportCallbackEXT(Instance, &createInfo, nullptr, callback_.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug callback!");
		}
	}

	void VulkanRenderSystem::CreateSurface() const {
		if (glfwCreateWindowSurface(Instance, pGLFWwindow, nullptr, Surface.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void VulkanRenderSystem::PickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				PhysicalDevice = device;
				break;
			}
		}

		if (PhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void VulkanRenderSystem::CreateLogicalDevice() {
		QueueFamilyIndices indices = FindQueueFamilies(PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

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

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(PhysicalDevice, &createInfo, nullptr, Device.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device_!");
		}

		vkGetDeviceQueue(Device, indices.graphicsFamily, 0, &GraphicsQueue);
		vkGetDeviceQueue(Device, indices.presentFamily, 0, &PresentQueue);
	}

	void VulkanRenderSystem::CreateSwapChain() {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(PhysicalDevice);

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

		QueueFamilyIndices indices = FindQueueFamilies(PhysicalDevice);
		uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.presentFamily) };

		if (indices.graphicsFamily != indices.presentFamily) {
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
		if (vkCreateSwapchainKHR(Device, &createInfo, nullptr, &newSwapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		SwapChain = newSwapChain;

		vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, nullptr);
		swapChainImages_.resize(imageCount);
		vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, swapChainImages_.data());

		SwapChainImageFormat = surfaceFormat.format;
		SwapChainExtent = extent;
	}

	void VulkanRenderSystem::CreateImageViews() {
		swapChainImageViews_.resize(swapChainImages_.size(), VkCom<VkImageView>{Device, vkDestroyImageView});

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

		if (vkCreateRenderPass(Device, &renderPassInfo, nullptr, RenderPass.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanRenderSystem::CreateCommandPool() {
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(PhysicalDevice);
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

		if (vkCreateCommandPool(Device, &poolInfo, nullptr, PrimaryCommandPool.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics command pool!");
		}
	}

	void VulkanRenderSystem::CreateDepthResources() {
		VkFormat depthFormat = FindDepthFormat();
		CreateImage(SwapChainExtent.width, SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage_, depthImageMemory_);
		CreateImageView(depthImage_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView_);
		TransitionImageLayout(depthImage_, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void VulkanRenderSystem::CreateFramebuffers() {
		swapChainFramebuffers_.resize(swapChainImageViews_.size(), VkCom<VkFramebuffer>{Device, vkDestroyFramebuffer});
		for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				swapChainImageViews_[i],
				depthImageView_
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = RenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = SwapChainExtent.width;
			framebufferInfo.height = SwapChainExtent.height;
			framebufferInfo.layers = 1;
			if (vkCreateFramebuffer(Device, &framebufferInfo, nullptr, swapChainFramebuffers_[i].replace()) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	VulkanDrawable* VulkanRenderSystem::GetDrawable() {
		if (Drawables.size() == currentDrawable_) {
			Drawables.resize(currentDrawable_ + 1);
		}
		VulkanDrawable* drawable = &Drawables[currentDrawable_];
		currentDrawable_++;
		return drawable;
	}

	void VulkanRenderSystem::CreateCommandBuffers() {
		if (PrimaryCommandBuffer.size() > 0) {
			vkFreeCommandBuffers(Device, PrimaryCommandPool, static_cast<uint32_t>(PrimaryCommandBuffer.size()), PrimaryCommandBuffer.data());
		}

		PrimaryCommandBuffer.resize(swapChainFramebuffers_.size());
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = PrimaryCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(PrimaryCommandBuffer.size());
		if (vkAllocateCommandBuffers(Device, &allocInfo, PrimaryCommandBuffer.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < PrimaryCommandBuffer.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			vkBeginCommandBuffer(PrimaryCommandBuffer[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = RenderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers_[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = SwapChainExtent;

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(PrimaryCommandBuffer[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = {
			0.0f,
			0.0f,
			static_cast<float>(SwapChainExtent.width),
			static_cast<float>(SwapChainExtent.height),
			0.0f,
			1.0f };

			vkCmdSetViewport(PrimaryCommandBuffer[i], 0, 1, &viewport);

			VkRect2D scissor = {
			{ 0, 0 },
			SwapChainExtent };

			vkCmdSetScissor(PrimaryCommandBuffer[i], 0, 1, &scissor);

			for (VulkanDrawable& drawable : Drawables) {
				drawable.RecordDrawCommand(PrimaryCommandBuffer[i]);
			}

			vkCmdEndRenderPass(PrimaryCommandBuffer[i]);

			if (vkEndCommandBuffer(PrimaryCommandBuffer[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void VulkanRenderSystem::CreateSemaphores() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(Device, &semaphoreInfo, nullptr, imageAvailableSemaphore_.replace()) != VK_SUCCESS ||
			vkCreateSemaphore(Device, &semaphoreInfo, nullptr, renderFinishedSemaphore_.replace()) != VK_SUCCESS) {

			throw std::runtime_error("failed to create semaphores!");
		}
	}

	void VulkanRenderSystem::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkCom<VkImageView>& imageView) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(Device, &viewInfo, nullptr, imageView.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}

	void VulkanRenderSystem::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkCom<VkImage>& image, VkCom<VkDeviceMemory>& imageMemory) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(Device, &imageInfo, nullptr, image.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(Device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(Device, &allocInfo, nullptr, imageMemory.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(Device, image, imageMemory, 0);
	}

	void VulkanRenderSystem::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = BeginCommandBuffer();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (HasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		} else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		} else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndCommandBuffer(commandBuffer);
	}

	void VulkanRenderSystem::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkCom<VkBuffer>& buffer, VkCom<VkDeviceMemory>& bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(Device, &bufferInfo, nullptr, buffer.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(Device, &allocInfo, nullptr, bufferMemory.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(Device, buffer, bufferMemory, 0);
	}

	void VulkanRenderSystem::CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = BeginCommandBuffer();

		VkImageSubresourceLayers subResource;
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;

		VkImageCopy region;
		region.srcSubresource = subResource;
		region.dstSubresource = subResource;
		region.srcOffset = { 0, 0, 0 };
		region.dstOffset = { 0, 0, 0 };
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;

		vkCmdCopyImage(
			commandBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region
		);

		EndCommandBuffer(commandBuffer);
	}

	void VulkanRenderSystem::CopyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = BeginCommandBuffer();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndCommandBuffer(commandBuffer);
	}

	void VulkanRenderSystem::CreateShaderModule(const std::vector<char>& code, VkCom<VkShaderModule>& shaderModule) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned(code.size() / 4 + 1);
		memcpy(codeAligned.data(), code.data(), code.size());

		createInfo.pCode = codeAligned.data();

		if (vkCreateShaderModule(Device, &createInfo, nullptr, shaderModule.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}
}
