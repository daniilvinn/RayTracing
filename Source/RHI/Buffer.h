#pragma once

#include <Core/Common.h>
#include <RHI/Utils.h>
#include <RHI/RHI.h>

#include <vk_mem_alloc.h>

namespace RT {

	enum class BufferFlags : u64
	{
		VERTEX_RATE = BIT(0),
		INSTANCE_RATE = BIT(1),
		INDEX_TYPE_UINT8 = BIT(2),
		INDEX_TYPE_UINT16 = BIT(3),
		INDEX_TYPE_UINT32 = BIT(4),
		CREATE_STAGING_BUFFER = BIT(5),
	};

	enum class BufferUsage : u32 {
		VERTEX_BUFFER,
		INDEX_BUFFER,
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		STAGING_BUFFER,
		SHADER_DEVICE_ADDRESS,
		INDIRECT_PARAMS
	};

	enum class BufferMemoryUsage : u32 {
		READ_BACK,
		SEQUENTIAL_WRITE,
		NO_HOST_ACCESS
	};

	struct BufferDesc
	{
		u64 Size;
		u64 Flags;
		BufferUsage Usage;
		BufferMemoryUsage MemoryUsage;
		MemoryResidency Residency;
	};

	class Buffer {
	public:
		Buffer(const BufferDesc& Desc);
		Buffer(const BufferDesc& Desc, void* Data, u64 DataSize);
		~Buffer();

		VkBuffer Raw() const { return m_Buffer; }

		BufferDesc GetDesc() const { return Desc; }
		u64 GetDeviceAddress();
		u64 GetPerFrameSize() { return Desc.Size / RHI::Instance->GetConfig().FramesInFlight; }
		u64 GetFrameOffset() { return 0 * GetPerFrameSize(); } // Todo
		VmaAllocation RawAllocation() const { return m_Allocation; }

		void UploadData(u64 Offset, void* Data, u64 DataSize);
		//void CopyRegionTo(Ref<DeviceCmdBuffer> cmd_buffer, Ptr<Buffer> dst_buffer, uint64 src_offset, uint64 dst_offset, uint64 size) override;
		//void Clear(Ref<DeviceCmdBuffer> cmd_buffer, uint64 offset, uint64 size, uint32 value) override;

	private:
		VkBuffer m_Buffer;
		VmaAllocation m_Allocation;

		BufferDesc Desc;

	};

}