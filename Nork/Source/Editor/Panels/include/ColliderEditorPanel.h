#pragma once

#include "ViewportPanel.h"
#include "../../Views/include/Viewport.h"

namespace Nork::Editor {
	class ColliderEditorPanel : public ViewportPanel
	{
	public:
		ColliderEditorPanel(Entity& ent);
		~ColliderEditorPanel();
		virtual void Content() override;
	private:
		Components::Collider collider;
		Entity& ent;
	};
}