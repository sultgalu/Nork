#pragma once

#include "Buffer.h"

namespace Nork::Renderer {

	class BufferBuilder
	{
	public:
		BufferBuilder& Data(void* data, size_t size)
		{
			this->data = data;
			this->size = size;
			return *this;
		}
		BufferBuilder& Usage(BufferUsage usage)
		{
			this->usage = usage;
			return *this;
		}
		BufferBuilder& Target(BufferTarget target)
		{
			this->target = target;
			return *this;
		}
		std::shared_ptr<Buffer> Create();
	private:
		void Validate()
		{
			if (usage == BufferUsage::None
				|| target == BufferTarget::None)
			{
				std::abort();
			}
		}
	private:
		GLuint handle;
		void* data;
		size_t size;
		BufferUsage usage = BufferUsage::None;
		BufferTarget target = BufferTarget::None;
	};
}