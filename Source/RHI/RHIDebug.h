#pragma once

#include "Core/Logging.h"
#include "Core/ICoreSystem.h"

#include <vulkan/vulkan.h>

namespace RT {

	class RHIDebug : public ICoreSystem {
	public:
		void Initialize(void* InputData) override;
		void Shutdown() override;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugLog(
			VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
			VkDebugUtilsMessageTypeFlagsEXT Type,
			const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
			void* UserData
		);

		static std::string VulkanResultToString(VkResult Result);

	private:
		VkInstance Instance;
		VkDebugUtilsMessengerEXT Logger;

	};

}

#if !IN_RELEASE_BUILD
#define VK_CHECK_RESULT(RHICall)																								\
			{ VkResult Result = RHICall;																						\
			if(Result != VK_SUCCESS) {																							\
				LOG_CRITICAL("RHI call failed ({0}): {1} ({2})", RHIDebug::VulkanResultToString(Result), __FILE__, __LINE__); 	\
			}}
#else
#define VK_CHECK_RESULT(fn) fn
#endif