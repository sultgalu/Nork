#include "LightState.h"
#include "../../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	LightState::LightState()
	{
		mainUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 4 * sizeof(uint32_t)).Create();
		mainUbo->BindBase(0);
		dlUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(DirLight)).Create();
		dlUbo->BindBase(1);
		dsUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(DirShadow)).Create();
		dsUbo->BindBase(2);
		plUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 1024 * sizeof(PointLight)).Create();
		psUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(PointShadow)).Create();
		plUbo->BindBase(3);
		psUbo->BindBase(4);
	}
	void LightState::Upload()
	{
		dlUbo->Bind().SetData(dirLights.data(), dirLights.size() * sizeof(DirLight));
		dsUbo->Bind().SetData(dirShadows.data(), dirShadows.size() * sizeof(DirShadow));
		plUbo->Bind().SetData(pointLights.data(), pointLights.size() * sizeof(PointLight));
		psUbo->Bind().SetData(pointShadows.data(), pointShadows.size() * sizeof(PointShadow));

		uint32_t counts[4];
		counts[0] = dirLights.size();
		counts[1] = dirShadows.size();
		counts[2] = pointLights.size();
		counts[3] = pointShadows.size();

		mainUbo->Bind().SetData(counts, sizeof(counts));
	}
}