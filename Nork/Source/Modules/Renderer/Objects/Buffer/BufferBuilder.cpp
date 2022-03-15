#include "BufferBuilder.h"
#include "../GLManager.h"

namespace Nork::Renderer {
	std::shared_ptr<Buffer> BufferBuilder::Create()
	{
		Validate();

		glGenBuffers(1, &handle);
		glBindBuffer(static_cast<GLenum>(target), handle);
		glBufferData(static_cast<GLenum>(target), size, data, static_cast<GLenum>(usage));
		Logger::Info("Created buffer ", handle);

		auto buffer = std::make_shared<Buffer>(handle, size, usage, target);
		GLManager::Get().buffers[buffer->GetHandle()] = buffer;
		return buffer;
	}

}