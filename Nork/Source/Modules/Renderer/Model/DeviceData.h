#pragma once

#include "../DeviceResource.h"

namespace Nork::Renderer {

template<class T>
struct DeviceDataProxy
{
	struct Writer
	{
		Writer(T& hostData, std::shared_ptr<BufferElement<T>> deviceData)
			: hostData(hostData), copy(hostData), deviceData(deviceData)
		{}
		~Writer()
		{
			if (std::memcmp(&copy, &hostData, sizeof(T)))
				*deviceData = hostData;
		}
		T* operator->()
		{
			return &hostData;
		}
		T& hostData;
		T copy;
		std::shared_ptr<BufferElement<T>> deviceData;
	};

	DeviceDataProxy(const std::shared_ptr<BufferElement<T>>& deviceData)
		: deviceData(deviceData)
	{
		// default-initialize on device too
		*deviceData = hostData;
	}

	T hostData;
	std::shared_ptr<BufferElement<T>> deviceData;

	const T* operator->() const
	{
		return &hostData;
	}
	Writer operator->()
	{
		return Data();
	}
	Writer Data()
	{
		return Writer(hostData, deviceData);
	}
	const T& Data() const
	{
		return hostData;
	}
};
}