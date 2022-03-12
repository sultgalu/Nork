#pragma 
#include "../Objects/GLManager.h"
#include "../Model/Lights.h"

namespace Nork::Renderer {
	
	class LightStateSynchronizer
	{
	public:
		LightStateSynchronizer(LightState state = LightState()) : lightState(state)
		{
		}
		void Initialize()
		{
			mainUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 4 * sizeof(uint32_t)).Create();
			dlUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(DirLight)).Create();
			dsUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(DirShadow)).Create();
			plUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 1024 * sizeof(PointLight)).Create();
			psUbo = BufferBuilder().Usage(BufferUsage::StaticDraw).Target(BufferTarget::UBO).Data(nullptr, 10 * sizeof(PointShadow)).Create();
			mainUbo->BindBase(0);
			dlUbo->BindBase(1);
			dsUbo->BindBase(2);
			plUbo->BindBase(3);
			psUbo->BindBase(4);
		}
		void Synchronize()
		{
			dlUbo->Bind().SetData(lightState.dirLights.data(), lightState.dirLights.size() * sizeof(DirLight));
			dsUbo->Bind().SetData(lightState.dirShadows.data(), lightState.dirShadows.size() * sizeof(DirShadow));
			plUbo->Bind().SetData(lightState.pointLights.data(), lightState.pointLights.size() * sizeof(PointLight));
			psUbo->Bind().SetData(lightState.pointShadows.data(), lightState.pointShadows.size() * sizeof(PointShadow));
			
			uint32_t counts[4];
			counts[0] = lightState.dirLights.size();
			counts[1] = lightState.dirShadows.size();
			counts[2] = lightState.pointLights.size();
			counts[3] = lightState.pointShadows.size();

			mainUbo->Bind().SetData(counts, sizeof(counts));
		}
		LightState& GetLightState() { return lightState; }
	private:
		LightState lightState;
		std::shared_ptr<Buffer> mainUbo, plUbo, psUbo, dlUbo, dsUbo;
	};
}