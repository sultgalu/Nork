#pragma once

#include "Vulkan/Buffer.h"
#include "MemoryAllocator.h"
#include "Commands.h"
#include "BufferCopy.h"

namespace Nork::Renderer {
	class HostVisibleBuffer;
	class Buffer
	{
	public:
		Buffer(Buffer&&) = default;
		Buffer(std::shared_ptr<Vulkan::Buffer> buffer, DeviceMemory memory)
			: underlying(buffer), memory(memory)
		{}
		static std::shared_ptr<Buffer> Create(vk::BufferUsageFlags usage,
			vk::DeviceSize size, const MemoryFlags& memFlags);
		static std::shared_ptr<HostVisibleBuffer> CreateHostVisible(vk::BufferUsageFlags usage,
			vk::DeviceSize size, const MemoryFlags& memFlags);

		void Write(const void* data)
		{
			Write(data, memory.Size(), 0);
		}
		void Read(vk::DeviceSize offset, vk::DeviceSize size, const std::function<void(std::span<std::byte>)>& cb) {
			Download(offset, size, cb); // this should be the same as Write, we should collect reads and flush them once
		}
		// Overwrite data previously written by 'Write' method. 'pos' is returned by 'Write'
		void OverWrite(const void* data, vk::DeviceSize size, uint32_t pos); 
		uint32_t Write(const void* data, vk::DeviceSize size, vk::DeviceSize offset);
		void FlushWrites(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess);
	protected:
		virtual void Download(vk::DeviceSize offset, vk::DeviceSize size, const std::function<void(std::span<std::byte>)>& cb) const {
			DownloadWithStagingBuffer(offset, size, cb);
		}
		void DownloadWithStagingBuffer(vk::DeviceSize offset, vk::DeviceSize size, const std::function<void(std::span<std::byte>)>& cb) const;
		virtual void Upload(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess)
		{
			UploadWithStagingBuffer(syncStages, syncAccess);
		}
		void UploadWithStagingBuffer(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess);
	public:
		auto Underlying() const {
			return underlying;
		}
	public:
		std::vector<BufferCopy> writes;
		std::vector<uint8_t> writeData;

		std::shared_ptr<Vulkan::Buffer> underlying;
		DeviceMemory memory;
	};

	class HostVisibleBuffer : public Buffer
	{
	protected:
		using Buffer::Buffer;
		void Upload(vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess) override
		{
			UploadWithMemcpy();
		}
		void UploadWithMemcpy();
		void Download(vk::DeviceSize offset, vk::DeviceSize size, const std::function<void(std::span<std::byte>)>& cb) const override {
			cb(Data());
		}
	public:
		/*template<class T = void>
		T* Ptr() const {
			return reinterpret_cast<T*>(memory.Ptr());
		}
		template<class T = void>
		T* Ptr() {
			return reinterpret_cast<T*>(memory.Ptr());
		}*/
		template<class T = std::byte>
		auto Data() const {
			return std::span<T>((T*)memory.Ptr(), (T*)memory.Ptr() + memory.Size() / sizeof(T));
		}
		template<class T = std::byte>
		auto Data() {
			return std::span<T>((T*)memory.Ptr(), (T*)memory.Ptr() + memory.Size() / sizeof(T));
		}
	};

	template<class T>
	struct BufferPointer
	{
		Buffer* buffer;
		uint32_t offset; // count
		BufferPointer() = default;
		BufferPointer(const BufferPointer&) = delete;
	protected:
		void Read(uint32_t offsetCount, uint32_t count, const std::function<void(std::span<T>)>& cb) {
			buffer->Read((offset + offsetCount) * sizeof(T), count * sizeof(T), [cb](std::span<std::byte> span) {
				cb(std::span((T*)span.data(), span.size_bytes() / sizeof(T)));
			});
		}
		void Write(const T* data, uint32_t count, uint32_t offsetCount) {
			this->buffer->Write(data, count * sizeof(T), (offset + offsetCount) * sizeof(T));
		}
	};
	template<class T>
	struct BufferElement : BufferPointer<T>
	{
		using BufferPointer<T>::BufferPointer;
		void operator=(const T& val)
		{
			Write(val);
		}
		virtual void Write(const T& val)
		{
			BufferPointer<T>::Write(&val, 1, 0);
		}
		void Read(const std::function<void(std::span<T>)>& cb) {
			BufferPointer<T>::Read(0, 1, cb);
		}
	};
	template<class T>
	struct BufferView : BufferPointer<T>
	{
		const uint32_t count;
		BufferView(uint32_t count)
			: count(count)
		{}
		void Write(const T* data, uint32_t count, uint32_t offsetCount = 0)
		{
			if (count > this->count)
				std::unreachable();
			BufferPointer<T>::Write(data, count, offsetCount);
		}
		void Read(uint32_t offsetCount, uint32_t count, const std::function<void(std::span<T>)>& cb) {
			if (count > this->count)
				std::unreachable();
			BufferPointer<T>::Read(offsetCount, count, cb);
		}
		void ReadAll(const std::function<void(std::span<T>)>& cb) {
			Read(0, count, cb);
		}
		size_t SizeBytes() {
			return sizeof(T) * count;
		}
		// BufferElement<T> operator[](const size_t& idx)
		// {
		// 	return BufferElement<T>(this->buffer, (this->offset + idx) * sizeof(T));
		// }
	};
}