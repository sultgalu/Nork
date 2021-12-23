#pragma once

namespace Nork::Renderer
{
	enum class ObjectType
	{
		Buffer
	};

	template<ObjectType Type>
	struct ObjectHandle
	{
		ObjectHandle()
		{
			using enum ObjectType;
			if constexpr (Type == Buffer)
				glGenBuffers(1, &handle);
		}
		~ObjectHandle()
		{
			using enum ObjectType;
			if constexpr (Type == Buffer)
				glDeleteBuffers(1, &handle);
		}
		inline GLuint Get() { return handle; }
	private:
		GLuint handle = 0;
	};

	template<ObjectType Type>
	using ObjectHandleRef2 = std::unique_ptr<ObjectHandle<Type>>;

	template<ObjectType Type>
	class ObjectHandleRef
	{
	public:
		ObjectHandleRef()
		{
			ptr = std::make_unique<ObjectHandle<Type>>();
		}
		inline GLuint Get()
		{
			return ptr.get()->Get();
		}
	private:
		std::shared_ptr<ObjectHandle<Type>> ptr;
	};

	enum class BufferTarget : GLenum
	{
		Vertex = GL_ARRAY_BUFFER, Index = GL_ELEMENT_ARRAY_BUFFER,
		VertexArray = GL_VERTEX_ARRAY
	};
	enum class IndexedBufferTarget : GLenum
	{
		SSBO = GL_SHADER_STORAGE_BUFFER, UBO = GL_UNIFORM_BUFFER,
		AtomicCounter = GL_ATOMIC_COUNTER_BUFFER
	};

	enum class BufferUsage
	{
		DynamicDraw = GL_DYNAMIC_DRAW, StaticDraw = GL_STATIC_DRAW
	};

	template<GLenum Target, class T>
	class Buffer
	{
	public:
		static constexpr auto target = Target;

		Buffer(size_t size = 0, BufferUsage usage = BufferUsage::StaticDraw)
			: usage(static_cast<GLenum>(usage))
		{
			Bind();
			Allocate(size);
		}
		Buffer(std::span<T> data, BufferUsage usage = BufferUsage::StaticDraw)
			: usage(usage)
		{
			Bind();
			Allocate(data);
		}
		inline void Bind()
		{
			/*if (current != handle.Get())
			{
				current = handle.Get();
				glBindBuffer(target, current);
			}*/
			glBindBuffer(target, handle.Get());
		}
		inline void SetData(const std::span<T> data, size_t offset = 0)
		{
			SetData(data.data(), data.size(), offset);
		}
		inline void SetData(const T* data, size_t count, size_t offset = 0)
		{
			if (this->count < count + offset)
			{
				if (offset > 0)
					std::abort();
				else Allocate(data, count);
			}
			glBufferSubData(target, offset * sizeof(T), sizeof(T) * count, data);
		}
		inline void Resize(size_t count)
		{
			if (this->count != count)
			{
				Allocate(count);
			}
		}
		inline void Allocate(size_t count, const T* data = nullptr)
		{
			glBufferData(target, sizeof(T) * count, data, usage);
			this->count = count;
		}
		inline void Allocate(const std::span<T> data)
		{
			Allocate(data.size(), data.data());
		}
		inline void Allocate(const T* data, size_t count)
		{
			Allocate(count, data);
		}
		inline void GetData(const T* data, size_t count, size_t offset = 0)
		{
			glGetBufferSubData(target, offset * sizeof(T), count * sizeof(T), data);
		}
		inline void GetData(const std::span<T> data, size_t offset = 0)
		{
			glGetBufferSubData(target, offset * sizeof(T), data.size_bytes(), data.data());
		}
	protected:
		size_t count = 0;

		GLenum usage;
		ObjectHandleRef<ObjectType::Buffer> handle;
		inline static GLuint current;
	};

	template<BufferTarget Target, class T>
	class SingleBindPointBuffer : public Buffer<static_cast<GLenum>(Target), T>
	{
		
	};
	template<IndexedBufferTarget Target, class T>
	class MultiBindPointBuffer : public Buffer<static_cast<GLenum>(Target), T>
	{
		static constexpr auto target = static_cast<GLenum>(Target);
	public:
		MultiBindPointBuffer(uint8_t bindigIndex, size_t size = 0, BufferUsage usage = BufferUsage::StaticDraw)
			: Buffer<static_cast<GLenum>(Target), T>(size, usage), index(bindigIndex)
		{
			this->Bind();
			BindBase();
		}
		template<class T>
		MultiBindPointBuffer(uint8_t bindigIndex, std::span<T> data, BufferUsage usage = BufferUsage::StaticDraw)
			: Buffer<static_cast<GLenum>(Target), T>(data, usage), index(bindigIndex)
		{
			this->Bind();
			BindBase();
		}
	private:
		void BindBase()
		{
			glBindBufferBase(target, index, this->handle.Get());
		}
	private:
		uint8_t index;
	};

	template<class T>
	using VBO = SingleBindPointBuffer<BufferTarget::Vertex, T>;
	template<class T>
	using IBO = SingleBindPointBuffer<BufferTarget::Index, T>;
	template<class T>
	using VAO = SingleBindPointBuffer<BufferTarget::VertexArray, T>;

	template<class T>
	using SSBO = MultiBindPointBuffer<IndexedBufferTarget::SSBO, T>;
	template<class T>
	using UBO = MultiBindPointBuffer<IndexedBufferTarget::UBO, T>;
	template<class T>
	using AtomicBuffer = MultiBindPointBuffer<IndexedBufferTarget::AtomicCounter, T>;
}