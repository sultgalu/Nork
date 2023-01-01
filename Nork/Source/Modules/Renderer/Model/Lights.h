#pragma once

#include "../Data/Lights.h"
#include "DeviceData.h"

namespace Nork::Renderer {

struct DirLight: DeviceDataProxy<Data::DirLight>
{
	using DeviceDataProxy<Data::DirLight>::DeviceDataProxy;
};
struct PointLight : DeviceDataProxy<Data::PointLight>
{
	using DeviceDataProxy<Data::PointLight>::DeviceDataProxy;
};
}