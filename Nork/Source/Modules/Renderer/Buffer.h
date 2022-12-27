#pragma once

#include "Vulkan/Buffer.h"
#include "Commands.h"

namespace Nork::Renderer {
	struct BufferCopy; // avoid circular dep.

	class HostWritableBuffer;
	class Buffer
	{
	public:
		Buffer(Buffer&&) = default;
		Buffer(std::shared_ptr<Vulkan::Buffer> buffer, DeviceMemory memory)
			: underlying(buffer), memory(memory)
		{}
		static std::shared_ptr<Buffer> Create(vk::BufferUsageFlags usage,
			vk::DeviceSize size, const MemoryFlags& memFlags);
		static std::shared_ptr<HostWritableBuffer> CreateHostWritable(vk::BufferUsageFlags usage,
			vk::DeviceSize size, const MemoryFlags& memFlags);

		void Write(const void* data)
		{
			Write(data, memory.Size(), 0);
		}
		void Write(const void* data, vk::DeviceSize size, vk::DeviceSize offset);
		void FlushWrites(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess);
	protected:
		virtual void Upload(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess)
		{
			UploadWithStagingBuffer(syncStages, syncAccess);
		}
		void UploadWithStagingBuffer(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess);
	public:
		std::shared_ptr<Vulkan::Buffer>& Underlying()
		{
			return underlying;
		}
	public:
		std::vector<BufferCopy> writes;
		std::vector<uint8_t> writeData;

		std::shared_ptr<Vulkan::Buffer> underlying;
		DeviceMemory memory;
	};

	class HostWritableBuffer : public Buffer
	{
	protected:
		using Buffer::Buffer;
		void Upload(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess) override
		{
			UploadWithMemcpy();
		}
		void UploadWithMemcpy();
	public:
		template<class T = void>
		T* Ptr()
		{
			if (!memory.IsMapped())
				memory.Map();
			return reinterpret_cast<T*>(memory.Ptr());
		}
	};

}