#pragma once

import Nork.Renderer;

namespace Nork::Renderer {
	class BufferWrapper
	{
	public:
		BufferWrapper(BufferTarget, uint32_t stride = 1, size_t initialCount = 0);
		std::shared_ptr<void*> Add(const void*, size_t count);
		void* GetPtr();
		size_t FreeSpaceKeepOrder();
		size_t FreeSpace();

		std::shared_ptr<void*> Front() { return pointers.front(); };
		std::shared_ptr<void*> Back() { return pointers.back(); }
		void Erase(size_t);
		void Swap(size_t, size_t);

		size_t GetSize() { return count * stride; }
		size_t GetCount() { return count; }
		std::shared_ptr<Buffer> GetBuffer() { return buffer; }
	private:
		std::vector<std::shared_ptr<void*>> pointers;
		std::shared_ptr<Buffer> buffer;
		size_t count = 0;
		const uint32_t stride;
	};
}