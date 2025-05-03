#pragma once

#include "Core/Common.h"
#include <vulkan/vulkan.h>
#include <volk.h>
#include <vector>

struct GLFWwindow;

namespace RT {

	struct SwapchainConfig {
		u32 Width;
		u32 Height;
		GLFWwindow* NativeWindow;
	};

	class RHISwapchain {
	public:
		RHISwapchain(const SwapchainConfig& InConfig);
		~RHISwapchain();

		void Resize(u32 NewWidth, u32 NewHeight);
		void Present(VkQueue PresentQueue, u32 ImageIndex, VkSemaphore WaitSemaphore);
		VkResult AcquireNextImage(VkSemaphore ImageAvailableSemaphore, u32* OutImageIndex);

		// Getters
		VkFormat GetImageFormat() const { return ImageFormat; }
		VkExtent2D GetExtent() const { return Extent; }
		const std::vector<VkImage>& GetImages() const { return Images; }
		const std::vector<VkImageView>& GetImageViews() const { return ImageViews; }
		VkSwapchainKHR GetHandle() const { return Swapchain; }

	private:
		void InitSurface();
		void CreateSwapchain();
		void CleanupSwapchain();

	private:
		SwapchainConfig Config = {};
		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkSwapchainKHR Swapchain = VK_NULL_HANDLE;

		VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D Extent = {};

		std::vector<VkImage> Images;
		std::vector<VkImageView> ImageViews;

		VkDevice Device = VK_NULL_HANDLE;      
		VkInstance Instance = VK_NULL_HANDLE;  
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	};

}
