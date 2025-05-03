// VulkanSyncObjects.hpp
#pragma once

#include <RHI/RHI.h>
#include <RHI/Utils.h>
#include <Core/Logging.h>

#include <volk.h>

namespace RT {

	class Fence {
	public:
		Fence(bool CreateSignaled = false)
			: Handle(VK_NULL_HANDLE)
		{
			DeviceHandle = RHI::Instance->GetDevice();

			VkFenceCreateInfo CreateInfo{};
			CreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			CreateInfo.flags = CreateSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

			if (auto Result = vkCreateFence(DeviceHandle, &CreateInfo, nullptr, &Handle); Result != VK_SUCCESS) {
				LOG_CRITICAL("Failed to create fence; Code: {}", VkResultToString(Result));
				Handle = VK_NULL_HANDLE;
			}
		}

		~Fence() {
			if (Handle != VK_NULL_HANDLE) {
				vkDestroyFence(DeviceHandle, Handle, nullptr);
			}
		}

		Fence(const Fence&) = delete;
		Fence& operator=(const Fence&) = delete;

		bool Wait(uint64_t Timeout = UINT64_MAX) const {
			VkResult Result = vkWaitForFences(DeviceHandle, 1, &Handle, VK_TRUE, Timeout);
			return Result == VK_SUCCESS;
		}

		bool Reset() const {
			return vkResetFences(DeviceHandle, 1, &Handle) == VK_SUCCESS;
		}

		VkFence Get() const {
			return Handle;
		}

	private:
		VkDevice DeviceHandle;
		VkFence Handle;
	};

	class Semaphore {
	public:
		Semaphore()
			: Handle(VK_NULL_HANDLE)
		{
			DeviceHandle = RHI::Instance->GetDevice();

			VkSemaphoreCreateInfo CreateInfo{};
			CreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			CreateInfo.pNext = nullptr;
			CreateInfo.flags = 0;

			if (auto Result = vkCreateSemaphore(DeviceHandle, &CreateInfo, nullptr, &Handle); Result != VK_SUCCESS) {
				LOG_CRITICAL("Failed to create semaphore; Code: {}", VkResultToString(Result));
				Handle = VK_NULL_HANDLE;
			}
		}

		~Semaphore() {
			if (Handle != VK_NULL_HANDLE) {
				vkDestroySemaphore(DeviceHandle, Handle, nullptr);
			}
		}

		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;

		VkSemaphore Get() const {
			return Handle;
		}

	private:
		VkDevice DeviceHandle;
		VkSemaphore Handle;
	};

}