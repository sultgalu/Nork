#pragma once
#include "../Objects/Buffer/Buffer.h"
#include "../Model/Lights.h"

namespace Nork::Renderer2 {
	
	class LightStateSynchronizer
	{
	public:
		LightStateSynchronizer(LightState state = LightState()) : lightState(state)
		{
		}
		void Initialize()
		{
			mainUbo.Create().Bind(BufferTarget::UBO).BindBase(0).Allocate(4 * sizeof(uint32_t));
			dlUbo.Create().Bind(BufferTarget::UBO).BindBase(1).Allocate(10 * sizeof(DirLight));
			dsUbo.Create().Bind(BufferTarget::UBO).BindBase(2).Allocate(10 * sizeof(DirShadow));
			plUbo.Create().Bind(BufferTarget::UBO).BindBase(3).Allocate(1024 * sizeof(PointLight));
			psUbo.Create().Bind(BufferTarget::UBO).BindBase(4).Allocate(10 * sizeof(PointShadow));
		}
		void Synchronize()
		{
			dlUbo.Bind(BufferTarget::UBO).SetData(lightState.dirLights.data(), lightState.dirLights.size() * sizeof(DirLight));
			dsUbo.Bind(BufferTarget::UBO).SetData(lightState.dirShadows.data(), lightState.dirShadows.size() * sizeof(DirShadow));
			plUbo.Bind(BufferTarget::UBO).SetData(lightState.pointLights.data(), lightState.pointLights.size() * sizeof(PointLight));
			psUbo.Bind(BufferTarget::UBO).SetData(lightState.pointShadows.data(), lightState.pointShadows.size() * sizeof(PointShadow));
			
			uint32_t counts[4];
			counts[0] = lightState.dirLights.size();
			counts[1] = lightState.dirShadows.size();
			counts[2] = lightState.pointLights.size();
			counts[3] = lightState.pointShadows.size();

			mainUbo.Bind(BufferTarget::UBO).SetData(counts, sizeof(counts));
		}
		LightState& GetLightState() { return lightState; }
	private:
		LightState lightState;
		Buffer mainUbo, plUbo, psUbo, dlUbo, dsUbo;
	};
}