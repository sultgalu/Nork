#pragma once

#include "../Objects/Buffer/Buffer.h"

namespace Nork::Renderer {
	class BufferWrapper
	{
	public:
		BufferWrapper(BufferTarget, uint32_t stride = 1, size_t initialCount = 0);
		std::shared_ptr<size_t> Add(const void*, size_t count);
		void Update(std::shared_ptr<size_t> idx, const void*, size_t count);
		void* GetPtr();
		size_t FreeSpace(bool shrinkToFit = false);
		std::shared_ptr<Buffer> GetBuffer() { return buffer; }

		size_t GetSize() { return count * stride; }
		size_t GetCount() { return count; }
	private:
		std::shared_ptr<Buffer> CreateBuffer(size_t count, BufferTarget target);
	private:
		std::vector<std::shared_ptr<size_t>> indexes;
		std::shared_ptr<Buffer> buffer;
		size_t count = 0;
		const uint32_t stride;
	};
}