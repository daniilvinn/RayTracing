#pragma once

#include <Core/Common.h>
#include <RHI/Buffer.h>

#include <vk_mem_alloc.h>

namespace RT {

	struct VulkanAllocatorStatistics
	{
		VulkanAllocatorStatistics()
			: Allocated(0)
			, Freed(0)
			, CurrentlyAllocated(0)
			, NumActiveImages(0)
			, NumActiveBuffers(0)
		{}

		u64 Allocated;
		u64 Freed;
		u64 CurrentlyAllocated;
		u64 NumActiveImages;
		u64 NumActiveBuffers;
	};

	class VulkanMemoryAllocator {
	public:
		static void Init();

		VmaAllocation AllocateBuffer(const BufferDesc& Spec, VkBufferCreateInfo* CreateInfo, u32 Flags, VkBuffer* Buffer);
		VmaAllocation AllocateImage(VkImageCreateInfo* CreateInfo, u32 Flags, VkImage* Image);

		void DestroyBuffer(VkBuffer Buffer, VmaAllocation Allocation);
		void DestroyImage(VkImage Image, VmaAllocation Allocation);

		void* MapMemory(VmaAllocation Allocation);
		void UnmapMemory(VmaAllocation Allocation);

		void InvalidateAllocation(VmaAllocation Allocation, u64 Size = VK_WHOLE_SIZE, u64 Offset = 0);

		VulkanAllocatorStatistics GetStats() const { return Statistics; }

	public:
		static VulkanMemoryAllocator* Instance;

	private:
		VulkanMemoryAllocator();

	private:
		VmaAllocator Allocator;
		VulkanAllocatorStatistics Statistics;
	};
	
	inline VulkanMemoryAllocator* GRHIAlloc = nullptr;

}