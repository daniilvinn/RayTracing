#include <RHI/CmdList.h>
#include <RHI/RHI.h>

#include <volk.h>

namespace RT {

	CmdList::CmdList(bool Transient)
		: Device(RHI::Instance->GetDevice())
	{
		VkCommandPoolCreateInfo PoolCreateInfo = {};
		PoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		PoolCreateInfo.queueFamilyIndex = RHI::Instance->GetGraphicsQueueIndex();
		PoolCreateInfo.flags = Transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;

		vkCreateCommandPool(Device, &PoolCreateInfo, nullptr, &CommandPool);
		
		VkCommandBufferAllocateInfo CmdListAllocateInfo = {};
		CmdListAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CmdListAllocateInfo.commandPool = CommandPool;
		CmdListAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CmdListAllocateInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(Device, &CmdListAllocateInfo, &CmdBuffer);

		VkCommandBufferBeginInfo BeginInfo = {};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.flags = Transient ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

		vkBeginCommandBuffer(CmdBuffer, &BeginInfo);
	}

	CmdList::~CmdList()
	{
		vkDestroyCommandPool(Device, CommandPool, nullptr);
	}

}