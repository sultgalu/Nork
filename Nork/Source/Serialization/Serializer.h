#pragma once

#include "Modules/ECS/Storage.h"

namespace Nork::Serialization
{
	void Init();
	void SerializeRegistry(const ECS::Registry& reg, std::ostream& s);
	void DeserializeRegistry(ECS::Registry& reg, std::istream& s);
}