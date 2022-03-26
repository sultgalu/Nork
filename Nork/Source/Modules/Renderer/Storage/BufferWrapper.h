#pragma once

#include "../Objects/Buffer/Buffer.h"

namespace Nork::Renderer {
	class BufferWrapper
	{
	public:
		BufferWrapper(BufferTarget, uint32_t stride = 1, size_t initialCount = 0);
		std::shared_ptr<size_t> Add(const void*, size_t count);
		void Update(std::shared_ptr<size_t> idx, const void*, size_t count);
		void Prepare(size_t);
		size_t FreeSpace(bool shrinkToFit = false);
		std::shared_ptr<Buffer> GetBuffer() { return buffer; }
	private:
		std::vector<std::shared_ptr<size_t>> indexes;
		std::shared_ptr<Buffer> buffer;
		const uint32_t stride;
	};
}