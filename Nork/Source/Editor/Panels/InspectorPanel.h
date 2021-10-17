#pragma once
#include "Base/Panel.h"
#include "Components/All.h"

namespace Nork::Editor
{
	using namespace Components;
	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel(EditorData& d)
			: Panel("Inspector", d), scene(data.engine.scene)
		{
		} //, reg(data.engine.scene.registry.GetUnderlying()) { }
	protected:
		virtual void DrawContent() override;
	private:
		void CompAdder();
		template<typename T>
		void CompSelector();
		void CameraComp(Camera*);
		void TransformComp(Transform*);
		void ModelComp(Model*);
		void PointLighComp(PointLight*);
		void DirLightComp(DirLight*, DirShadow*);
		void DirShadowComp(DirShadow*, DirLight*);
		void NameComp(Tag*);
	private:
		//entt::registry& reg;
		Scene::Scene& scene;
	};
}