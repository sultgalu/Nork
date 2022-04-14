#pragma once

#include "Base/Panel.h"

namespace Nork::Editor2
{
	class HierarchyPanel : public Panel
	{
	public:
		HierarchyPanel (EditorData& d)
			: Panel("Hierarchy", d), reg(data.engine.scene.registry) { }
	protected:
		void DrawContent() override;
	private:
		entt::registry& reg;
	};
}