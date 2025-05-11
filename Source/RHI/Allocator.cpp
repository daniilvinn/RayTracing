#include <RHI/Allocator.h>

namespace RT {

	void VulkanMemoryAllocator::Init()
	{
		GRHIAlloc = new VulkanMemoryAllocator;
	}

}