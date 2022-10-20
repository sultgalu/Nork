export module Nork.Renderer:BufferBuilder;

export import :Buffer;

export namespace Nork::Renderer {

	class BufferBuilder
	{
	public:
		BufferBuilder& Data(void* data, size_t size)
		{
			this->data = data;
			this->size = size;
			return *this;
		}
		BufferBuilder& Target(BufferTarget target)
		{
			this->target = target;
			return *this;
		}
		BufferBuilder& Flags(BufferStorageFlags flags)
		{
			this->flags |= flags;
			return *this;
		}
		std::shared_ptr<Buffer> Create();
		std::shared_ptr<MutableBuffer> CreateMutable(BufferUsage);
		std::shared_ptr<Buffer> CreateCopy(std::shared_ptr<Buffer>, size_t size);
	private:
		void Validate()
		{
			if (flags & BufferStorageFlags::Persistent &&
				((static_cast<BufferAccess>(flags) & BufferAccess::ReadWrite) == BufferAccess::None))
			{
				std::abort();
			}
			if (target == BufferTarget::None
				|| size == 0)
			{
				std::abort();
			}
		}
		void ValidateMutable()
		{
			if (target == BufferTarget::None)
			{
				std::abort();
			}
		}
	private:
		GLuint handle;
		void* data;
		size_t size;
		BufferTarget target = BufferTarget::None;
		BufferStorageFlags flags = BufferStorageFlags::None;
	};
}