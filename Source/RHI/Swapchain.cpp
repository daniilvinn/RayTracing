#include <Core/Common.h>
#include <RHI/Swapchain.h>

#include <Core/Application.h>
#include <RHI/RHI.h>
#include <RHI/CmdList.h>

#include <GLFW/glfw3.h>

namespace RT {

	Swapchain::Swapchain(const SwapchainDesc& spec)
		: Surface(VK_NULL_HANDLE), SwapchainHandle(VK_NULL_HANDLE), Desc(spec)
	{
		SwapchainHandle = new VkSwapchainKHR(VK_NULL_HANDLE);
	}

	Swapchain::~Swapchain()
	{
		for (auto& image : Images) {
			Application::Instance->EnqueueObjectFinalize(
				[vk_view = image->RawView()]() {
					vkDestroyImageView(RHI::Instance->GetDevice(), vk_view, nullptr);
				}
			);
		}
	}

	void Swapchain::CreateSurface(const SwapchainDesc& Desc)
	{
		VkDevice LogicalDevice = RHI::Instance->GetDevice();
		VkPhysicalDevice PhysicalDevice = RHI::Instance->GetPhysicalDevice();

		VK_CHECK_RESULT(
			glfwCreateWindowSurface(
				RHI::Instance->GetInstance(),
				(GLFWwindow*)Desc.MainWindow->Raw(),
				nullptr,
				&Surface
			)
		);


		Application::Instance->EnqueueObjectFinalize(
			[CapturedSurface = Surface]() {
				vkDestroySurfaceKHR(RHI::Instance->GetInstance(), CapturedSurface, nullptr);
			}
		);

		// Looking for available presentation modes.
		// FIFO (aka v-synced) is guaranteed to be available, so only looking for Mailbox (non v-synced) mode.
		u32 PresentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr);

		std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data());

		SupportsMailboxPresentation = false;
		for (auto& mode : PresentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) SupportsMailboxPresentation = true;
		}

		if (!SupportsMailboxPresentation && !Desc.VSync)
			LOG_WARN("Mailbox presentation mode is requested but not supporeted. Falling back to FIFO mode");

		// Looking for suitable surface color space
		u32 SurfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice()->Raw(), Surface, &SurfaceFormatCount, nullptr);

		std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice()->Raw(), Surface, &SurfaceFormatCount, SurfaceFormats.data());

		// To begin, set surface format as first available, if no srgb 32-bit format is supported.
		SurfaceFormat = SurfaceFormats[0];

		for (auto& Format : SurfaceFormats) {
			if (Format.format == VK_FORMAT_B8G8R8A8_UNORM && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				SurfaceFormat = Format;
			}
		}

		Application::Instance->EnqueueObjectFinalize(
			[vk_device = device->Raw(), CapturedSwapchain = SwapchainHandle]() {
				vkDestroySwapchainKHR(vk_device, *CapturedSwapchain, nullptr);
			}
		);

	}

	void Swapchain::CreateSwapchain(const SwapchainDesc& InDesc)
	{
		VkDevice device = VulkanGraphicsContext::Get()->GetDevice();

		m_Specification = InDesc;
		Images.reserve(ImageCount);
		m_Semaphores.reserve(InDesc.frames_in_flight);

		if (*SwapchainHandle) [[likely]]
		{
			vkDestroySwapchainKHR(device->Raw(), *SwapchainHandle, nullptr);
		}

		uvec2 extent = (uvec2)InDesc.extent;
		if (extent.x + extent.y == 0) {
			extent = { 1, 1 };
		}

		VkSurfaceCapabilitiesKHR surface_capabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDevice()->Raw(), Surface, &surface_capabilities);

		VkSwapchainCreateInfoKHR swapchain_create_info = {};
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface = Surface;
		swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
		swapchain_create_info.imageFormat = SurfaceFormat.format;
		swapchain_create_info.imageColorSpace = SurfaceFormat.colorSpace;
		swapchain_create_info.minImageCount = 3;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.imageExtent = { extent.x, extent.y };
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.preTransform = surface_capabilities.currentTransform;
		swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

		if (SupportsMailboxPresentation) {
			swapchain_create_info.presentMode = m_Specification.vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
		}

		VK_CHECK_RESULT(vkCreateSwapchainKHR(device->Raw(), &swapchain_create_info, nullptr, SwapchainHandle));

		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device->Raw(), *SwapchainHandle, (uint32*)&ImageCount, nullptr));

		Images.reserve(ImageCount);
		std::vector<VkImage> pure_swapchain_images(ImageCount);

		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device->Raw(), *SwapchainHandle, (uint32*)&ImageCount, pure_swapchain_images.data()));

		VulkanDeviceCmdBuffer image_layout_transition_command_buffer = device->AllocateTransientCmdBuffer();

		for (auto& image : Images) {
			RuntimeExecutionContext::Get().GetObjectLifetimeManager().EnqueueObjectDeletion(
				[vk_device = device->Raw(), vk_view = image->RawView()]() {
					vkDestroyImageView(vk_device, vk_view, nullptr);
				}
			);
		}

		Images.clear();

		for (auto& image : pure_swapchain_images) {
			VkImageView image_view = VK_NULL_HANDLE;

			VkImageViewCreateInfo image_view_create_info = {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image = image;
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = SurfaceFormat.format;
			image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(device->Raw(), &image_view_create_info, nullptr, &image_view));

			ImageSpecification swapchain_image_spec = {};
			swapchain_image_spec.extent = { (uint32)m_Specification.extent.x, (uint32)m_Specification.extent.y, 1 };
			swapchain_image_spec.usage = ImageUsage::RENDER_TARGET;
			swapchain_image_spec.type = ImageType::TYPE_2D;
			swapchain_image_spec.format = convert(SurfaceFormat.format);

			Images.push_back(CreateRef<VulkanImage>(&g_PersistentAllocator, swapchain_image_spec, image, image_view));

			VkImageMemoryBarrier image_memory_barrier = {};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.image = image;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_memory_barrier.subresourceRange.baseArrayLayer = 0;
			image_memory_barrier.subresourceRange.layerCount = 1;
			image_memory_barrier.subresourceRange.baseMipLevel = 0;
			image_memory_barrier.subresourceRange.levelCount = 1;

			vkCmdPipelineBarrier(
				image_layout_transition_command_buffer,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&image_memory_barrier
			);
		}

		device->ExecuteTransientCmdBuffer(image_layout_transition_command_buffer, true);

		for (auto& image : Images) {
			image->SetCurrentLayout(ImageLayout::PRESENT_SRC);
		}

		m_CurrentFrameIndex = 0;

		/*  ===================
		*	Create sync objects
		*/

		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Only create sync objects once
		if (m_Semaphores.size() == 0) {
			for (int32 i = 0; i < m_Specification.frames_in_flight; i++) {
				VkSemaphore render_semaphore;
				VkSemaphore present_semaphore;

				VK_CHECK_RESULT(vkCreateSemaphore(device->Raw(), &semaphore_create_info, nullptr, &render_semaphore));
				VK_CHECK_RESULT(vkCreateSemaphore(device->Raw(), &semaphore_create_info, nullptr, &present_semaphore));

				m_Semaphores.push_back({ render_semaphore, present_semaphore });

				RuntimeExecutionContext::Get().GetObjectLifetimeManager().EnqueueCoreObjectDelection(
					[vk_device = device->Raw(), render_semaphore, present_semaphore]() {
						vkDestroySemaphore(vk_device, render_semaphore, nullptr);
						vkDestroySemaphore(vk_device, present_semaphore, nullptr);
					}
				);

			}

			m_Fences.reserve(InDesc.frames_in_flight);

			m_Semaphores.shrink_to_fit();
			m_Fences.shrink_to_fit();

			VkFenceCreateInfo fence_create_info = {};
			fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (int32 i = 0; i < InDesc.frames_in_flight; i++) {
				VkFence fence;
				VK_CHECK_RESULT(vkCreateFence(device->Raw(), &fence_create_info, nullptr, &fence));
				m_Fences.push_back(fence);

				RuntimeExecutionContext::Get().GetObjectLifetimeManager().EnqueueCoreObjectDelection(
					[vk_device = device->Raw(), fence]() {
						vkDestroyFence(vk_device, fence, nullptr);
					}
				);

			}
		}

		OMNIFORCE_CORE_INFO(
			"Created window swapchain with extent: {0}x{1}, VSync: {2}, image count: {3}",
			InDesc.extent.x,
			InDesc.extent.y,
			InDesc.vsync ? "on" : "off",
			ImageCount
		);
	}

	void Swapchain::SetVSync(bool vsync)
	{
		if (m_Specification.vsync == vsync) return;

		m_Specification.vsync = vsync;

		SwapchainSpecification current_spec = GetSpecification();
		SwapchainSpecification new_spec = current_spec;
		new_spec.vsync = vsync;

		CreateSwapchain(new_spec);
	}

	void Swapchain::BeginFrame()
	{
		auto device = VulkanGraphicsContext::Get()->GetDevice();

		VK_CHECK_RESULT(vkWaitForFences(device->Raw(), 1, &m_Fences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(vkResetFences(device->Raw(), 1, &m_Fences[m_CurrentFrameIndex]));

		VkResult acquisition_result = vkAcquireNextImageKHR(
			device->Raw(),
			*SwapchainHandle,
			UINT64_MAX,
			m_Semaphores[m_CurrentFrameIndex].present_complete,
			VK_NULL_HANDLE,
			&CurrentImageIndex
		);

		if (acquisition_result == VK_ERROR_OUT_OF_DATE_KHR || acquisition_result == VK_SUBOPTIMAL_KHR)
		{
			VkSurfaceCapabilitiesKHR surface_capabilities = {};
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDevice()->Raw(), Surface, &surface_capabilities);

			SwapchainSpecification new_spec = GetSpecification();
			new_spec.extent = { (int32)surface_capabilities.currentExtent.width, (int32)surface_capabilities.currentExtent.height };

			vkQueueWaitIdle(device->GetAsyncComputeQueue());

			CreateSwapchain(new_spec);
		}

	}

	void Swapchain::EndFrame()
	{
		auto device = VulkanGraphicsContext::Get()->GetDevice();

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pImageIndices = &CurrentImageIndex;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = SwapchainHandle;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &m_Semaphores[m_CurrentFrameIndex].render_complete;
		present_info.pResults = nullptr;



		VkResult present_result = vkQueuePresentKHR(device->GetGeneralQueue(), &present_info);

		if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
		{
			VkSurfaceCapabilitiesKHR surface_capabilities = {};
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDevice()->Raw(), Surface, &surface_capabilities);

			SwapchainSpecification new_spec = GetSpecification();
			new_spec.extent = { (int32)surface_capabilities.currentExtent.width, (int32)surface_capabilities.currentExtent.height };

			vkQueueWaitIdle(device->GetGeneralQueue());

			CreateSwapchain(new_spec);
		}

		m_CurrentFrameIndex = (CurrentImageIndex + 1) % m_Specification.frames_in_flight;
	}

}