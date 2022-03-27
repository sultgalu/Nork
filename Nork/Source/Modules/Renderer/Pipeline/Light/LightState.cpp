#include "LightState.h"
#include "../../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	LightState::LightState()
	{
		using enum BufferStorageFlags;
		auto flags = WriteAccess | Persistent | Coherent;
		mainUbo = BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, 4 * sizeof(uint32_t)).Create();
		dlUbo =	BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(DirLight)).Create();
		dsUbo = BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(DirShadow)).Create();
		plUbo = BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, 1024 * sizeof(PointLight)).Create();
		psUbo = BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(PointShadow)).Create();
		mainUbo->BindBase(0).Map(BufferAccess::Write);
		dlUbo->BindBase(1).Map(BufferAccess::Write);
		dsUbo->BindBase(2).Map(BufferAccess::Write);
		plUbo->BindBase(3).Map(BufferAccess::Write);
		psUbo->BindBase(4).Map(BufferAccess::Write);
	}
	void LightState::Upload()
	{
		std::memcpy(dlUbo->GetPersistentPtr(), dirLights.data(), dirLights.size() * sizeof(DirLight));
		std::memcpy(dsUbo->GetPersistentPtr(), dirShadows.data(), dirShadows.size() * sizeof(DirShadow));
		std::memcpy(plUbo->GetPersistentPtr(), pointLights.data(), pointLights.size() * sizeof(PointLight));
		std::memcpy(psUbo->GetPersistentPtr(), pointShadows.data(), pointShadows.size() * sizeof(PointShadow));

		uint32_t counts[4];
		counts[0] = dirLights.size();
		counts[1] = dirShadows.size();
		counts[2] = pointLights.size();
		counts[3] = pointShadows.size();

		std::memcpy(mainUbo->GetPersistentPtr(), counts, sizeof(counts));
	}
}