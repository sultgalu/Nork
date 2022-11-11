#pragma once

#include "../Objects/Buffer/Buffer.h"

namespace Nork::Renderer {
	template<class T>
	class SmartMappedBuffer
	{
	public:
		struct Region
		{
			bool operator==(const Region& other) const { return index == other.index; }
			bool IsEmpty() const { return index == nullptr; }
			uint32_t Index() const { return *index; }
		protected:
			Region() = default;
			Region(std::shared_ptr<uint32_t> index, uint32_t count)
				: index(index), count(count)
			{}
			std::shared_ptr<uint32_t> index;
			uint32_t count;
			friend SmartMappedBuffer;
			friend T* SmartMappedBuffer::Data();
		};
		struct Element : public Region
		{
			Element() = default;
			Element(SmartMappedBuffer<T>& buffer, std::shared_ptr<uint32_t> index)
				: Region(index, 1), buffer(&buffer)
			{
				Get() = T();
			}
			T* operator->() { return &Get(); }
			const T* operator->() const { return &Get(); }
			T& operator*() { return Get(); }
			const T& operator*() const { return Get(); }
			T& Get() { return buffer->Data()[*Region::index]; }
			const T& Get() const { return buffer->Data()[*Region::index]; }
		private:
			SmartMappedBuffer<T>* buffer;
		};
		struct Array : public Region
		{
			Array() = default;
			Array(SmartMappedBuffer<T>& buffer, std::shared_ptr<uint32_t> index, uint32_t count)
				: Region(index, count), buffer(&buffer)
			{}
			T& operator[](uint32_t i) { return Data()[i]; }
			const T& operator[](uint32_t i) const { return Data()[i]; }
			void CopyFrom(const T* src, size_t count, size_t offs = 0)
			{
				std::memcpy(Data() + offs, src, count * sizeof(T));
			}
			void CopyTo(T* dst, size_t count, size_t offs = 0) const
			{
				std::memcpy(dst, Data() + offs, count * sizeof(T));
			}
			size_t SizeBytes() const { return sizeof(T) * Region::count; }
			uint32_t Count() const { return Region::count; }
			T* Data() { return &buffer->Data()[*Region::index]; }
			const T* Data() const { return &buffer->Data()[*Region::index]; }
		private:
			SmartMappedBuffer<T>* buffer;
		};

		SmartMappedBuffer(BufferTarget, size_t initialCount);

		std::shared_ptr<Buffer> GetBuffer() { return buffer; }

		Element Allocate();
		Array Allocate(size_t count);

		size_t Count() const { return regions.empty() ? 0 : *regions.back().index + regions.back().count; }
		size_t Capacity() const { return buffer->GetSize(); };
		size_t SizeBytes() const { return Count() * sizeof(T); }
		size_t CapacityBytes() const { return Capacity() * sizeof(T); };

		size_t UnusedAllocatedCount();
		size_t UnusedCount() { return Capacity() - Count() - UnusedAllocatedCount(); };
		size_t UnusedAllocatedBytes() { return UnusedAllocatedCount() * sizeof(T); }
		size_t UnusedBytes() { return UnusedCount() * sizeof(T); };
		size_t UsedAllocatedCount() { return Count() - UnusedAllocatedCount(); }
		size_t UsedAllocatedBytes() { return UsedAllocatedCount() *sizeof(T); }
	
		bool DidCreateNewBuffer() { return onNewBuffer; }
	private:
		void NewCompactBuffer(size_t toBeAllocatedBytes);
		std::shared_ptr<Buffer> CreateBuffer(BufferTarget, size_t bytes);
		T* Data() { return (T*)buffer->GetPersistentPtr(); }
	private:
		std::vector<Region> regions;
		std::shared_ptr<Buffer> buffer;
		BufferTarget target;
		bool onNewBuffer = false;
	};

	template<class T, BufferTarget _Target>
	class TypedSmartMappedBuffer : public SmartMappedBuffer<T>
	{
	public:
		TypedSmartMappedBuffer(size_t initialCount = 0)
			: SmartMappedBuffer<T>(_Target, initialCount)
		{}
	};

	template<class T>
	using UBO = TypedSmartMappedBuffer<T, BufferTarget::UBO>;
	template<class T>
	using SSBO = TypedSmartMappedBuffer<T, BufferTarget::SSBO>;
	template<class T>
	using VBO = TypedSmartMappedBuffer<T, BufferTarget::Vertex>;
	using IBO = TypedSmartMappedBuffer<uint32_t, BufferTarget::Index>;
}