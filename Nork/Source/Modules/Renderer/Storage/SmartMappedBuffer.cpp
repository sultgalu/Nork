#include "SmartMappedBuffer.h"
#include "../Objects/Buffer/BufferBuilder.h"
#include "../Data/Vertex.h"
#include "../Data/Material.h"
#include "../Data/Lights.h"
#include "../Data/Lights.h"

namespace Nork::Renderer {
	template<class T>
	SmartMappedBuffer<T>::SmartMappedBuffer(BufferTarget target, size_t initialCount)
	{
		this->target = target;
		buffer = CreateBuffer(target, 1 * sizeof(T));
	}
	template<class T>
	std::shared_ptr<Buffer> SmartMappedBuffer<T>::CreateBuffer(BufferTarget target, size_t bytes)
	{
		if (bytes == 0)
			return nullptr;
		using enum BufferStorageFlags;
		auto buffer = BufferBuilder()
			.Flags(ReadAccess | WriteAccess | Persistent | Coherent)
			.Target(target)
			.Data(nullptr, bytes)
			.Create();
		buffer->Bind().Map(BufferAccess::ReadWrite);
		if (this->buffer != nullptr && this->buffer->GetBase() != Buffer::invalidBase)
			buffer->BindBase(this->buffer->GetBase());
		return buffer;
	}
	template<class T>
	void SmartMappedBuffer<T>::NewCompactBuffer(size_t toBeAllocatedBytes)
	{
		onNewBuffer = true;
		if (this->buffer != nullptr)
		{
			auto newBuf = CreateBuffer(this->target, toBeAllocatedBytes * 2);
		}

		size_t newSize = (UsedAllocatedBytes() + toBeAllocatedBytes) * 2;
		auto newBuf = CreateBuffer(buffer->GetTarget(), newSize);
		std::vector<Region> newRegions;
		newRegions.reserve(regions.size());

		T* oldPtr = (T*)buffer->GetPersistentPtr();
		T* newPtr = (T*)newBuf->GetPersistentPtr();
		size_t count = 0;
		for (Region& region : regions)
		{
			if (region.index.use_count() > 1)
			{
				*region.index = count;
				newRegions.push_back(region);
				count += region.count;
			}
			else
			{
				std::memcpy(newPtr, oldPtr, count * sizeof(T));
				newPtr += count;
				oldPtr += count + region.count;
				count = 0;
			}
		}
		if (count > 0)
		{
			std::memcpy(newPtr, oldPtr, count * sizeof(T));
		}

		regions = newRegions;
		buffer = newBuf;
	}
	template<class T>
	SmartMappedBuffer<T>::Element SmartMappedBuffer<T>::Allocate()
	{
		onNewBuffer = false;
		if (SizeBytes() + sizeof(T) > buffer->GetSize())
		{
			NewCompactBuffer(sizeof(T));
		}
		Element element(*this, std::make_shared<uint32_t>(Count()));
		regions.push_back(element);
		return element;
	}
	template<class T>
	SmartMappedBuffer<T>::Array SmartMappedBuffer<T>::Allocate(size_t count)
	{
		onNewBuffer = false;
		if (SizeBytes() + count * sizeof(T) > buffer->GetSize())
		{	
			NewCompactBuffer(count * sizeof(T));
		}
		Array array(*this, std::make_shared<uint32_t>(Count()), count);
		regions.push_back(array);
		return array;
	}
	template<class T>
	size_t SmartMappedBuffer<T>::UnusedAllocatedCount()
	{
		size_t result = 0;
		for (Region& region : regions)
		{
			if (region.index.use_count() == 1)
			{
				result += region.count;
			}
		}
		return result;
	}
	template SmartMappedBuffer<uint32_t>;
	template SmartMappedBuffer<Data::Vertex>;
	template SmartMappedBuffer<Data::Material>;
	template SmartMappedBuffer<Data::DirLight>;
	template SmartMappedBuffer<Data::PointLight>;
	template SmartMappedBuffer<Data::DirShadow>;
	template SmartMappedBuffer<Data::PointShadow>;
	template SmartMappedBuffer<glm::mat4>;
}

