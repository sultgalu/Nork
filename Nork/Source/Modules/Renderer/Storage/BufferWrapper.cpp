#include "BufferWrapper.h"
#include "../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	std::shared_ptr<Buffer> BufferWrapper::CreateBuffer(size_t count, BufferTarget target)
	{
		using enum BufferStorageFlags;
		auto buffer = BufferBuilder()
			.Flags(WriteAccess | Persistent | Coherent)
			.Target(target)
			.Data(nullptr, count * stride)
			.Create();
		buffer->Bind().Map(BufferAccess::Write);
		return buffer;
	}
	BufferWrapper::BufferWrapper(BufferTarget target, uint32_t stride, size_t initialCount)
		: stride(stride)
	{
		buffer = CreateBuffer(initialCount, target);
	}
	std::shared_ptr<size_t> BufferWrapper::Add(const void* data, size_t count)
	{
		if ((this->count + count) * stride > buffer->GetSize())
		{
			auto newBuffer = CreateBuffer((this->count + count) * stride * 2, buffer->GetTarget());
			std::memcpy(newBuffer->GetPersistentPtr(), buffer->GetPersistentPtr(), GetSize());
			// sync
			buffer = newBuffer;
		}
		auto index = std::make_shared<size_t>(GetCount());
		indexes.push_back(index);
		std::memcpy((char*)buffer->GetPersistentPtr() + GetSize(), data, count * stride);
		this->count += count;
		return index;
	}
	void BufferWrapper::Update(std::shared_ptr<size_t> idx, const void* data, size_t count)
	{
		std::memcpy((char*)buffer->GetPersistentPtr() + *idx * stride, data, count * stride);
	}
	void* BufferWrapper::GetPtr()
	{
		return buffer->GetPersistentPtr();
	}
	size_t BufferWrapper::FreeSpace(bool shrinkToFit)
	{
		std::abort();
		/*std::vector<std::pair<size_t, size_t>> eraseRanges;
		size_t freedSpace = 0;

		for (size_t i = 0; i < indexes.size(); i++)
		{
			if (indexes[i].use_count() <= 1)
			{
				size_t from = *indexes[i] * stride;
				size_t to = i + 1 < indexes.size() ?
					*indexes[i + 1] * stride : buffer->GetSize();

				if (eraseRanges.size() > 0 &&
					eraseRanges.back().first + eraseRanges.back().second == from)
				{
					eraseRanges.back().second += to - from;
				}
				else
				{
					eraseRanges.push_back({ from, to - from });
				}
				freedSpace += to - from;
			}
			else
			{
				*indexes[i] -= freedSpace;
			}
		}

		buffer->Bind().Erase(eraseRanges);

		if (shrinkToFit)
		{
			buffer->ShrinkToFit();
		}

		for (int i = (int)indexes.size() - 1; i >= 0; i--)
		{
			if (indexes[i].use_count() <= 1)
			{
				indexes.erase(indexes.begin() + i);
			}
		}

		return freedSpace;*/
	}
}

