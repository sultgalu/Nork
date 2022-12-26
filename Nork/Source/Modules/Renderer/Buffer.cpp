#include "Buffer.h"

#include "MemoryTransfer.h"

namespace Nork::Renderer {
	std::shared_ptr<Buffer> Buffer::Create(vk::BufferUsageFlagBits usage, vk::DeviceSize size, 
		vk::MemoryPropertyFlags memFlags, const std::vector<vk::MemoryPropertyFlags>& desiredMemFlags)
	{
		auto buffer = std::make_shared<Vulkan::Buffer>(Vulkan::BufferCreateInfo(size, usage));
		auto memreq = buffer->getMemoryRequirements();
		auto memory = DeviceMemory(
			MemoryAllocator::Instance().Allocate(memreq, memFlags, desiredMemFlags));

		buffer->BindMemory(memory.Underlying(), memory.PoolOffset());
		if (memory.IsHostVisible() && memory.IsHostCoherent())
			return std::make_shared<HostWritableBuffer>(buffer, memory);
		else
			return std::make_shared<Buffer>(buffer, memory);
	}
	std::shared_ptr<HostWritableBuffer> Buffer::CreateHostWritable(vk::BufferUsageFlagBits usage, vk::DeviceSize size,
		vk::MemoryPropertyFlags memFlags, const std::vector<vk::MemoryPropertyFlags>& desiredMemFlags)
	{
		auto buffer = Create(usage, size, memFlags, desiredMemFlags);
		if (buffer->memory.IsHostVisible() && buffer->memory.IsHostCoherent())
			return std::reinterpret_pointer_cast<HostWritableBuffer>(buffer);
		std::unreachable();
	}
	void Buffer::UploadWithStagingBuffer(const void* data, vk::DeviceSize size, vk::DeviceSize offset)
	{
		MemoryTransfer::Instance().CopyToBuffer(*this, data, size, offset);
	}
}