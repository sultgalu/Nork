#pragma once

#include "Vulkan/Buffer.h"

namespace Nork::Renderer {

	class HostWritableBuffer;
	class Buffer
	{
	public:
		Buffer(Buffer&&) = default;
		Buffer(std::shared_ptr<Vulkan::Buffer> buffer, DeviceMemory memory)
			: underlying(buffer), memory(memory)
		{}
		static std::shared_ptr<Buffer> Create(vk::BufferUsageFlagBits usage, vk::DeviceSize size, 
			vk::MemoryPropertyFlags memFlags, const std::vector<vk::MemoryPropertyFlags>& desiredMemFlags = {});
		static std::shared_ptr<HostWritableBuffer> CreateHostWritable(vk::BufferUsageFlagBits usage, vk::DeviceSize size,
			vk::MemoryPropertyFlags memFlags, const std::vector<vk::MemoryPropertyFlags>& desiredMemFlags = {});
		void Upload(const void* data)
		{
			Upload(data, memory.Size(), 0);
		}
		virtual void Upload(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0)
		{
			UploadWithStagingBuffer(data, size, offset);
		}
		void UploadWithStagingBuffer(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0);
		// template<class T>
		// void operator=(const T& val)
		// {
		// 	Upload(val, sizeof(val));
		// }
		std::shared_ptr<Vulkan::Buffer>& Underlying()
		{
			return underlying;
		}
	public:
		std::shared_ptr<Vulkan::Buffer> underlying;
		DeviceMemory memory;
	};

	class HostWritableBuffer : public Buffer
	{
	public:
		using Buffer::Buffer;
		// HostWritableBuffer(std::shared_ptr<Vulkan::Buffer> buffer, DeviceMemory memory)
		// 	: Buffer(buffer, memory)
		// {}
		void Upload(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0) override
		{
			UploadWithMemcpy(data, size, offset);
		}
		void UploadWithMemcpy(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0)
		{
			std::memcpy((uint8_t*)memory.Ptr() + offset, data, size);
		}
		template<class T = void>
		T* Ptr()
		{
			if (!memory.IsMapped())
				memory.Map();
			return reinterpret_cast<T*>(memory.Ptr());
		}
	public:
	};

}