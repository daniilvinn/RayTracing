#pragma once

#include <volk.h>

namespace RT {

	class CmdList
	{
	public:
		CmdList(bool Transient = false);
		~CmdList();

		VkCommandBuffer Get() const { return CmdBuffer; }

	private:
		VkDevice Device;
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer CmdBuffer = VK_NULL_HANDLE;
	};


}

