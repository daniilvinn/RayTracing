#pragma once

#include "RHIDebug.h"
#include "Core/Application.h"

#include <volk.h>

namespace RT {

	void RHIDebug::Initialize(void* InputData)
	{
		Instance = static_cast<VkInstance>(InputData);

		VkDebugUtilsMessengerCreateInfoEXT VulkanDebugCallbackCreateInfo = {};
		VulkanDebugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		VulkanDebugCallbackCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		VulkanDebugCallbackCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		VulkanDebugCallbackCreateInfo.pfnUserCallback = RHIDebug::DebugLog;

		vkCreateDebugUtilsMessengerEXT(Instance, &VulkanDebugCallbackCreateInfo, nullptr, &Logger);

		Application::Instance->EnqueueObjectFinalize([InstanceCaptured = Instance, LoggerCaptured = Logger]() {
			vkDestroyDebugUtilsMessengerEXT(InstanceCaptured, LoggerCaptured, nullptr);
		});
	}

	void RHIDebug::Shutdown()
	{
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL RHIDebug::DebugLog(
		VkDebugUtilsMessageSeverityFlagBitsEXT Severity, 
		VkDebugUtilsMessageTypeFlagsEXT Type, 
		const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, 
		void* UserData
	) {
		switch (Severity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOG_INFO("[Vulkan]: {0}\n", CallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_WARN("[Vulkan]: {0}\n", CallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOG_ERROR("[Vulkan]: {0}\n", CallbackData->pMessage);
			return VK_TRUE;
		default:
			break;
		}

		return VK_FALSE;
	}

	std::string RHIDebug::VulkanResultToString(VkResult Result)
	{
		switch (Result)
		{
		case VK_SUCCESS:											return "VK_SUCCESS";
		case VK_NOT_READY:											return "VK_NOT_READY";
		case VK_TIMEOUT:											return "VK_TIMEOUT";
		case VK_EVENT_SET:											return "VK_EVENT_SET";
		case VK_EVENT_RESET:										return "VK_EVENT_RESET";
		case VK_INCOMPLETE:											return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:							return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:							return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:						return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:									return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:							return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:							return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:						return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:							return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:							return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:								return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:							return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:								return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:										return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY:							return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:						return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION:								return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:				return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_PIPELINE_COMPILE_REQUIRED:							return "VK_PIPELINE_COMPILE_REQUIRED";
		case VK_ERROR_SURFACE_LOST_KHR:								return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:						return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:										return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:								return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:						return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:						return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV:							return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:	return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_NOT_PERMITTED_KHR:							return "VK_ERROR_NOT_PERMITTED_KHR";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR:									return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR:									return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR:								return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:							return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:					return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
		default:													std::unreachable();
		}
	}
}