#pragma once

#include <Core/Common.h>
#include <RHI/Sync.h>
#include <Core/Window.h>
#include <glm/glm.hpp>

namespace RT {

	struct SwapchainDesc {
		WindowConfig* MainWindow;
		glm::ivec2 Extent;
		i32 FramesInFlight;
		bool VSync;
		bool UseDepth;
	};

	class Swapchain {
	public:
		struct SwapchainSemaphores {
			Semaphore RenderComplete;
			Semaphore PresentComplete;
		};

		Swapchain(const SwapchainDesc& spec);
		~Swapchain();

		void CreateSurface(const SwapchainDesc& spec);
		void CreateSwapchain(const SwapchainDesc& spec);

		void BeginFrame();
		void EndFrame();

		VkSurfaceKHR RawSurface() const { return Surface; }
		VkSwapchainKHR RawSwapchain() const { return *SwapchainHandle; }

		bool IsVSync() const { return Desc.VSync; };
		void SetVSync(bool Value);

		SwapchainDesc GetSpecification() { return Desc; }
		u32 GetCurrentFrameIndex() const { return CurrentFrameIndex; }

		SwapchainSemaphores& GetSemaphores() { return Semaphores[CurrentFrameIndex]; }
		VkFence GetCurrentFence() const { return Fences[CurrentFrameIndex]; }
		Ptr<VkImage>& GetCurrentImage() { return Images[CurrentImageIndex]; };

	private:
		SwapchainDesc Desc;
		VkSurfaceKHR Surface;
		VkSwapchainKHR* SwapchainHandle; // pointer because we need to somehow access it on engine shutdown

		VkSurfaceFormatKHR SurfaceFormat;
		VkPresentModeKHR CurrentPresentMode;
		bool SupportsMailboxPresentation;

		std::vector<Ptr<VkImage>> Images;

		std::vector<SwapchainSemaphores> Semaphores;
		std::vector<VkFence> Fences;

		const u32 ImageCount = 3;

		u32 CurrentFrameIndex = 0;
		u32 CurrentImageIndex = 0;
	};

}