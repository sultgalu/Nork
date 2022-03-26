#include "BufferWrapper.h"
#include "../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	BufferWrapper::BufferWrapper(BufferTarget target, uint32_t stride, size_t initialCount)
		: stride(stride)
	{
		buffer = BufferBuilder()
			.Usage(BufferUsage::DynamicDraw)
			.Target(target)
			.Data(nullptr, initialCount * stride)
			.Create();
		Logger::Info("YEP CREATED ");
	}
	std::shared_ptr<size_t> BufferWrapper::Add(const void* data, size_t count)
	{
		auto index = std::make_shared<size_t>(buffer->GetSize() / stride);
		indexes.push_back(index);
		buffer->Bind().Append(data, count * stride);
		return index;
	}
	void BufferWrapper::Update(std::shared_ptr<size_t> idx, const void* data, size_t count)
	{
		buffer->Bind().SetData(data, stride * count, *idx * stride);
	}
	void BufferWrapper::Prepare(size_t count)
	{
		buffer->Bind().Reserve(buffer->GetSize() + count * stride);
	}
	size_t BufferWrapper::FreeSpace(bool shrinkToFit)
	{
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

		return freedSpace;
	}
}

