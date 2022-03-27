#include "BufferWrapper.h"
#include "../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	BufferWrapper::BufferWrapper(BufferTarget target, uint32_t stride, size_t initialCount)
		: stride(stride)
	{
		using enum BufferStorageFlags;
		buffer = BufferBuilder()
			.Flags(ReadAccess | WriteAccess | Persistent | Coherent)
			.Target(target)
			.Data(nullptr, initialCount * stride)
			.Create();
		buffer->Bind().Map(BufferAccess::ReadWrite);
	}
	std::shared_ptr<void*> BufferWrapper::Add(const void* data, size_t count)
	{
		if ((this->count + count) * stride > buffer->GetSize())
		{
			auto oldBasePtr = buffer->GetPersistentPtr();
			buffer = BufferBuilder().CreateCopy(buffer, (this->count + count) * 2 * stride); 
			auto newBasePtr = buffer->GetPersistentPtr();
			for (auto& ptr : pointers)
			{
				auto offset = (char*)*ptr - (char*)oldBasePtr;
				*ptr = (char*)newBasePtr + offset;
			}
		}
		auto ptr = std::make_shared<void*>((void*)((char*)buffer->GetPersistentPtr() + GetSize()));
		pointers.push_back(ptr);
		std::memcpy((char*)buffer->GetPersistentPtr() + GetSize(), data, count * stride);
		this->count += count;
		return ptr;
	}
	void* BufferWrapper::GetPtr()
	{
		return buffer->GetPersistentPtr();
	}
	size_t BufferWrapper::FreeSpaceKeepOrder()
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
	size_t BufferWrapper::FreeSpace()
	{
		std::abort();
	}
}

