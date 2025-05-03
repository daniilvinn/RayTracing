#pragma once

#include "Core/Common.h"
#include "Core/ICoreSystem.h"
#include "Core/Window.h"
#include "RHIExtensionList.h"
#include "RHIDebug.h"

#include <vulkan/vulkan.h>

namespace RT {

	struct RHIConfig {
		u32 FramesInFlight;
		Window* ApplicationWindow;
	};

	class RHI : public ICoreSystem 
	{
	public:
		void Initialize(void* InputData) override;
		void Shutdown() override;

		RHIConfig GetConfig() const {
			return Config;
		}

		VkDevice GetDevice() const { 
			return RenderDevice; 
		}

		VkPhysicalDevice GetPhysicalDevice() const {
			return PhysicalDevice;
		}

	public:
		static RHI* const Instance;

	private:
		void InitializeInstance();
		void InitializeDebugger();
		void InitializeRenderDevice();

		RHIExtensionList GetInstanceExtensionList();
		RHIExtensionList GetRenderDeviceExtensionList();
		bool CheckRenderDeviceSuitability(VkPhysicalDevice PhysicalDevice);
		void FetchQueueFamilyIndices();

	private:
		RHIConfig Config;

		VkInstance VulkanInstance = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties PhysicalDeviceProperties = {};
		VkDevice RenderDevice = VK_NULL_HANDLE;

		struct DeviceQueueFamilyIndices {
			u32 Graphics = -1;
			u32 AsyncCompute = -1;
			u32 AsyncTransfer = -1;
			u32 Present = -1;
		} QueueFamilyIndices;

		NO_RELEASE_ONLY(RHIDebug Debug);
	};

}