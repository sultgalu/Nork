#pragma once

#include "Buffer.h"

namespace Nork::Renderer {

template<class T>
struct BufferPointer
{
	Buffer* buffer;
	uint32_t offset;
	BufferPointer() = default;
	BufferPointer(const BufferPointer&) = delete;
};
template<class T>
struct BufferElement : BufferPointer<T>
{
	using BufferPointer<T>::BufferPointer;
	void operator=(const T& val)
	{
		Write(val);
	}
	virtual void Write(const T& val)
	{
		this->buffer->Write(&val, sizeof(T), this->offset * sizeof(T));
	}
};
template<class T>
struct BufferView : BufferPointer<T>
{
	const uint32_t count;
	BufferView(uint32_t count)
		: count(count)
	{}
	void Write(const T* data, uint32_t count, uint32_t offsetCount = 0)
	{
		if (count > this->count)
			std::unreachable();
		this->buffer->Write(data, count * sizeof(T), (this->offset + offsetCount) * sizeof(T));
	}
	// BufferElement<T> operator[](const size_t& idx)
	// {
	// 	return BufferElement<T>(this->buffer, (this->offset + idx) * sizeof(T));
	// }
};
template<class T>
class DeviceArrays
{
public:
	DeviceArrays(vk::BufferUsageFlags usage, const MemoryFlags& memFlags, uint32_t capacity,
	uint32_t maxFramesInFlight, vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess)
		: usage(usage |= vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst),
		memFlags(memFlags), capacity(capacity), syncStages(syncStages), syncAccess(syncAccess)
	{
		buffer = Buffer::Create(usage, CapacityBytes(), memFlags);
	}
	void FlushWrites()
	{
		buffer->FlushWrites(syncStages, syncAccess);
	}
	std::shared_ptr<BufferView<T>> New(uint32_t count)
	{
		auto arr = std::make_shared<BufferView<T>>(count);
		arr->buffer = buffer.get();
		arr->offset = size;

		size += count;
		return arr;
	}
public:
	vk::DeviceSize CapacityBytes() { return capacity * sizeof(T); }
	vk::DeviceSize SizeBytes() { return size * sizeof(T); }
public:
	std::vector<std::weak_ptr<BufferView<T>>> arrays;

	std::shared_ptr<Buffer> buffer;
	std::shared_ptr<Buffer> oldBuffer = nullptr;

	vk::PipelineStageFlags2 syncStages;
	vk::AccessFlags2 syncAccess;

	MemoryFlags memFlags;
	uint32_t capacity;
	uint32_t size = 0;
	vk::BufferUsageFlags usage;
};

template<class T>
class DeviceElements
{
public:
	struct BufferElementProxy : BufferElement<T>
	{
		DeviceElements<T>* allocator;
		void Write(const T& val) override
		{
			allocator->WriteElement(*this, val);
		}
		~BufferElementProxy()
		{
			allocator->OnElementDestroy(*this);
		}
	};
	DeviceElements(vk::BufferUsageFlags usage, const MemoryFlags& memFlags, uint32_t capacity,
				   uint32_t maxFramesInFlight, vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess)
		: usage(usage |= vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst),
		memFlags(memFlags), capacity(capacity), syncStages(syncStages), syncAccess(syncAccess)
	{
		buffer = Buffer::Create(usage, CapacityBytes(), memFlags);
		releasedElementsByFrame.resize(maxFramesInFlight - 1);
	}
	void Reserve(uint32_t newCapacity)
	{
		// oldBuffer = buffer;
		// buffer = Buffer::Create(usage, newSize, memFlags);
		// buffer->writes = std::move(oldBuffer->writes);
		// buffer->writeData = std::move(oldBuffer->writeData);
		// Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
		// {
		// 	auto barrier = vk::BufferMemoryBarrier2();
		// barrier.setBuffer(**oldBuffer->Underlying())
		// 	.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
		// 	.setDstAccessMask(vk::AccessFlagBits2::eTransferRead)
		// 	.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
		// 	.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
		// 	.setOffset(0).setSize(oldBuffer->memory.Size());
		// cmd.pipelineBarrier2(vk::DependencyInfo(vk::DependencyFlagBits::eByRegion).setBufferMemoryBarriers(barrier));
		// cmd.copyBuffer(**oldBuffer->Underlying(), **buffer->Underlying(), vk::BufferCopy(0, 0, size));
		// });
		// 
		// for (auto& el : elements)
		// {
		// 	el->buffer(buffer.get());
		// }
		// size = newSize;
	}
	void FlushWrites()
	{
		buffer->FlushWrites(syncStages, syncAccess);
		for (auto& [k, v] : allocatedIndexesInCurrentFrame)
			v = not_written_yet_pos;
	}
	std::shared_ptr<BufferElement<T>> New()
	{
		auto el = std::make_shared<BufferElementProxy>();
		el->allocator = this;
		Alloc(std::dynamic_pointer_cast<BufferElement<T>>(el));
		return el;
	}
	void Alloc(std::shared_ptr<BufferElement<T>> element)
	{
		if (!freeElements.empty())
		{
			uint32_t newIdx = *freeElements.begin();
			freeElements.erase(freeElements.begin());
			element->offset = newIdx;
			element->buffer = buffer.get();
			elements[newIdx] = element;
		}
		else
		{
			if (capacity <= size)
			{
				std::unreachable();
				// handle resize (to be a fraction of pool mem)
			}
			element->buffer = buffer.get();
			element->offset = size;
			elements.push_back(element);
			size++;
		}
		allocatedIndexesInCurrentFrame[element->offset] = not_written_yet_pos;
	}
	void WriteElement(BufferElement<T>& element, const T& val) // before write
	{
		if (allocatedIndexesInCurrentFrame.contains(element.offset))
		{ // element was moved here in this frame, can overwrite without affecting previos frames
			// no need to move
		}
		else
		{
			auto ptr = elements[element.offset].lock();
			elements[element.offset].reset();
			releasedElementsByFrame[0].push_back(element.offset);
			Alloc(ptr); // not_written_yet_pos will be true, could structure branch accordingly
		}
		if (allocatedIndexesInCurrentFrame[element.offset] == not_written_yet_pos)
		{ // element has not been written yet in current frame
			allocatedIndexesInCurrentFrame[element.offset] = // save the stagin buffer address for possible overwrite
				element.buffer->Write(&val, sizeof(T), element.offset * sizeof(T));
		}
		else
		{ // overwrite previous write from the same frame
			this->buffer->OverWrite(&val, sizeof(T), allocatedIndexesInCurrentFrame[element.offset]);
		}
	}
	void OnElementDestroy(BufferElement<T>& element)
	{
		elements[element.offset].reset();
		releasedElementsByFrame[0].push_back(element.offset);
	}
	void OnNewFrame()
	{ // if released element is the last one, this->size could be decreased instead
		for (auto& idx : releasedElementsByFrame.back())
			freeElements.insert(idx);
		for (size_t i = releasedElementsByFrame.size() - 1; i > 0; i--)
		{
			releasedElementsByFrame[i] = std::move(releasedElementsByFrame[i - 1]);
		}
		releasedElementsByFrame[0] = std::vector<uint32_t>();
		allocatedIndexesInCurrentFrame.clear();
	}
public:
	vk::DeviceSize CapacityBytes() { return capacity * sizeof(T); }
	vk::DeviceSize SizeBytes() { return size * sizeof(T); }
public:
	std::vector<std::weak_ptr<BufferElement<T>>> elements;

	std::shared_ptr<Buffer> buffer;
	std::shared_ptr<Buffer> oldBuffer = nullptr;

	std::vector<std::vector<uint32_t>> releasedElementsByFrame;
	std::set<uint32_t> freeElements;
	static constexpr uint32_t not_written_yet_pos = UINT32_MAX;
	std::unordered_map<uint32_t, uint32_t> allocatedIndexesInCurrentFrame;

	vk::PipelineStageFlags2 syncStages;
	vk::AccessFlags2 syncAccess;

	MemoryFlags memFlags;
	uint32_t capacity;
	uint32_t size = 0;
	vk::BufferUsageFlags usage;
};

}