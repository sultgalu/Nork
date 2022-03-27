#include "BufferBuilder.h"
#include "../GLManager.h"

namespace Nork::Renderer {
	std::shared_ptr<Buffer> BufferBuilder::Create()
	{
		Validate();

		glGenBuffers(1, &handle);
		auto buffer = std::make_shared<Buffer>(handle, size, target, flags);
		Logger::Info("Created buffer ", handle);
		GLManager::Get().buffers[buffer->GetHandle()] = buffer;

		buffer->Bind();
		glBufferStorage(static_cast<GLenum>(target), size, data, static_cast<GLenum>(flags));

		return buffer;
	}
	std::shared_ptr<MutableBuffer> BufferBuilder::CreateMutable(BufferUsage usage)
	{
		ValidateMutable();

		glGenBuffers(1, &handle);
		auto buffer = std::make_shared<MutableBuffer>(handle, 0, usage, target);
		Logger::Info("Created buffer ", handle);
		GLManager::Get().buffers[buffer->GetHandle()] = buffer;

		if (size > 0)
		{
			buffer->Bind().Allocate(size, data);
		}

		return buffer;
	}
}