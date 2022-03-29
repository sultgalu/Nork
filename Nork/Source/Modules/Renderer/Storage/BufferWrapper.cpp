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
		/*std::abort();
		std::vector<std::pair<size_t, size_t>> eraseRanges;
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
		std::abort();
	}
	size_t BufferWrapper::FreeSpace()
	{
		std::abort();
		/*char* ptr = (char*)buffer->GetPersistentPtr();
		uint32_t idx = 0;
		while (idx < count - 1)
		{
			if (pointers[idx].use_count() == 1)
			{
				auto copyTo = *pointers[idx];
				auto copyFrom = (count - 1) * stride;
				std::memcpy(ptr + copyTo, ptr + copyFrom, stride);
				count--;
			}
			idx++;
		}

		for (int i = pointers.size() - 1; i >= 0; i--)
		{
			if (pointers[i].use_count() == 1)
			{
				pointers.erase(pointers.begin() + i);
			}
		}*/
	}
	void BufferWrapper::Erase(size_t idx)
	{
		auto ptr = (char*)buffer->GetPersistentPtr();
		std::vector<char> copy(stride, 0);
		std::memcpy(copy.data(), *pointers[idx], stride);

		std::memmove(ptr + idx * stride, ptr + (idx + 1) * stride, stride);
		for (size_t i = idx + 1; i < pointers.size(); i++)
		{
			*((char*)*pointers[i]) -= stride;
		}

		*pointers[idx] = *pointers.back();
		std::memcpy(*pointers[idx], copy.data(), stride);

		auto temp = pointers[idx];
		pointers[idx] = pointers.back();
		pointers.back() = temp;
		count--;
		pointers.erase(pointers.end() - 1);
	}
	void BufferWrapper::Swap(size_t idx1, size_t idx2)
	{
		std::vector<char> copy(stride, 0);
		std::memcpy(copy.data(), *pointers[idx1], stride);
		std::memcpy(*pointers[idx1], *pointers[idx2], stride);
		std::memcpy(*pointers[idx2], copy.data(), stride);

		auto temp = *pointers[idx1];
		*pointers[idx1] = *pointers[idx2];
		*pointers[idx2] = temp;

		auto temp2 = pointers[idx1];
		pointers[idx1] = pointers[idx2];
		pointers[idx2] = temp2;
	}
}

