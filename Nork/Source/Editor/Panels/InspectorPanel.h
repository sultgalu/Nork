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
		void ModelComp(Drawable*);
		void PointLighComp(PointLight*);
		void DirLightComp(DirLight*);
		void KinematicComp(Kinematic*);
		void PolyComp(Polygon*);
		void ColliderComp(Collider*);
		//void asd(Polygon*);
		void NameComp(Tag*);
	private:
		//entt::registry& reg;
		Scene& scene;
	};
}