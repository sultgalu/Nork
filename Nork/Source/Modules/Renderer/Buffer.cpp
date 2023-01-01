#include "Buffer.h"

#include "MemoryTransfer.h"

namespace Nork::Renderer {
	std::shared_ptr<Buffer> Buffer::Create(vk::BufferUsageFlags usage, 
		vk::DeviceSize size, const MemoryFlags& memFlags)
	{
		auto buffer = std::make_shared<Vulkan::Buffer>(Vulkan::BufferCreateInfo(size, usage));
		auto memreq = buffer->getMemoryRequirements();
		auto memory = DeviceMemory(
			MemoryAllocator::Instance().Allocate(memreq, memFlags));

		buffer->BindMemory(memory.Underlying(), memory.PoolOffset());
		if (memory.IsHostVisible() && memory.IsHostCoherent())
			return std::make_shared<HostWritableBuffer>(buffer, memory);
		else
			return std::make_shared<Buffer>(buffer, memory);
	}
	std::shared_ptr<HostWritableBuffer> Buffer::CreateHostWritable(vk::BufferUsageFlags usage,
		vk::DeviceSize size, const MemoryFlags& memFlags)
	{
		auto buffer = Create(usage, size, memFlags);
		if (buffer->memory.IsHostVisible() && buffer->memory.IsHostCoherent())
			return std::reinterpret_pointer_cast<HostWritableBuffer>(buffer);
		std::unreachable();
	}
	void Buffer::OverWrite(const void* data, vk::DeviceSize size, uint32_t pos)
	{
		std::memcpy(&writeData[pos], data, size);
	}
	// TODO: writing to the same addresses should not be added but overwritten
	uint32_t Buffer::Write(const void* data, vk::DeviceSize size, vk::DeviceSize offset)
	{
		writes.push_back(BufferCopy{
			.size = size, .dstOffset = offset
			});
		auto pos = writeData.size();
		writeData.insert(writeData.end(), (uint8_t*)data, (uint8_t*)data + size);
		return pos;
	}
	void Buffer::FlushWrites(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess)
	{
		if (writes.size() == 0)
			return;
		vk::DeviceSize offset = 0;
		for (auto& write : writes)
		{
			write.data = &writeData[offset];
			offset += write.size;
		}

		Upload(syncStages, syncAccess);
		writes.clear();
		writeData.clear();
	}
	void Buffer::UploadWithStagingBuffer(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess)
	{
		MemoryTransfer::Instance().UploadToBuffer(*this, writes, syncStages, syncAccess);
	}
	void HostWritableBuffer::UploadWithMemcpy()
	{
		for (auto& write : writes)
		{
			std::memcpy((uint8_t*)memory.Ptr() + write.dstOffset, write.data, write.size);
		}
		if (!memory.IsHostCoherent())
		{
			std::vector<vk::MappedMemoryRange> ranges;
			ranges.reserve(writes.size());
			for (auto& write : writes)
			{
				ranges.emplace_back(**memory.Underlying(), memory.PoolOffset() + write.dstOffset, write.size);
			}
			Vulkan::Device::Instance().flushMappedMemoryRanges(ranges);
		}
	}
}