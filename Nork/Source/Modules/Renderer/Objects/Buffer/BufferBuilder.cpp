#include "BufferBuilder.h"
#include "../GLManager.h"

namespace Nork::Renderer {
	std::shared_ptr<Buffer> BufferBuilder::Create()
	{
		Validate();

		glGenBuffers(1, &handle);
		auto buffer = std::make_shared<Buffer>(handle, 0, 0, usage, target);
		Logger::Info("Created buffer ", handle);
		GLManager::Get().buffers[buffer->GetHandle()] = buffer;

		if (size > 0)
		{
			buffer->Bind().Allocate(size, data);
		}

		return buffer;
	}

}