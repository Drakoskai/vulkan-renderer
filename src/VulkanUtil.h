#ifndef VULKAN_UTL_H_
#define VULKAN_UTL_H_

#include "stdafx.h"
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanTypes.h"

namespace Vulkan {

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

#define VK_ERROR_TO_STRING(x) case static_cast<int>(x): return #x

	inline const char * VK_ErrorToString(VkResult result) {
		switch (result) {
			VK_ERROR_TO_STRING(VK_SUCCESS);
			VK_ERROR_TO_STRING(VK_NOT_READY);
			VK_ERROR_TO_STRING(VK_TIMEOUT);
			VK_ERROR_TO_STRING(VK_EVENT_SET);
			VK_ERROR_TO_STRING(VK_EVENT_RESET);
			VK_ERROR_TO_STRING(VK_INCOMPLETE);
			VK_ERROR_TO_STRING(VK_ERROR_OUT_OF_HOST_MEMORY);
			VK_ERROR_TO_STRING(VK_ERROR_OUT_OF_DEVICE_MEMORY);
			VK_ERROR_TO_STRING(VK_ERROR_INITIALIZATION_FAILED);
			VK_ERROR_TO_STRING(VK_ERROR_DEVICE_LOST);
			VK_ERROR_TO_STRING(VK_ERROR_MEMORY_MAP_FAILED);
			VK_ERROR_TO_STRING(VK_ERROR_LAYER_NOT_PRESENT);
			VK_ERROR_TO_STRING(VK_ERROR_EXTENSION_NOT_PRESENT);
			VK_ERROR_TO_STRING(VK_ERROR_FEATURE_NOT_PRESENT);
			VK_ERROR_TO_STRING(VK_ERROR_INCOMPATIBLE_DRIVER);
			VK_ERROR_TO_STRING(VK_ERROR_TOO_MANY_OBJECTS);
			VK_ERROR_TO_STRING(VK_ERROR_FORMAT_NOT_SUPPORTED);
			VK_ERROR_TO_STRING(VK_ERROR_SURFACE_LOST_KHR);
			VK_ERROR_TO_STRING(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
			VK_ERROR_TO_STRING(VK_SUBOPTIMAL_KHR);
			VK_ERROR_TO_STRING(VK_ERROR_OUT_OF_DATE_KHR);
			VK_ERROR_TO_STRING(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
			VK_ERROR_TO_STRING(VK_ERROR_VALIDATION_FAILED_EXT);
			VK_ERROR_TO_STRING(VK_ERROR_INVALID_SHADER_NV);
			VK_ERROR_TO_STRING(VK_RESULT_BEGIN_RANGE);
			VK_ERROR_TO_STRING(VK_RESULT_RANGE_SIZE);
		default: return "UNKNOWN";
		}
	}

	static bool VkCheck(VkResult ret) {
		if (ret != VK_SUCCESS) {
			DebugPrintF("VK: %s - %s", VK_ErrorToString(ret), ret);
			return false;
		}
		return true;
	}

	static void VkCheckOrThrow(VkResult ret, const char* format, ...) {
		if (!VkCheck(ret)) {
			va_list argList;
			va_start(argList, format);
			const unsigned int MAX_CHARS = 1024;
			static char s_buffer[MAX_CHARS];
			va_end(argList);
			throw std::runtime_error(s_buffer);
		}
	}

#define VK_FLAGS_NONE 0
#define DEFAULT_FENCE_TIMEOUT 100000000000

	template<typename A, typename B> constexpr auto Align(A offset, B alignment) {
		if (offset == 0 || alignment == 0) {
			return offset;
		}

		auto rest = offset % alignment;
		return rest ? offset + (alignment - rest) : offset;
	}

	std::string VkResultToString(VkResult result);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	std::vector<const char*> GetRequiredExtensions();
	bool HasStencilComponent(VkFormat format);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
}
#endif
