#pragma once

#include "ViewportPanel.h"
#include "../../Views/include/Viewport.h"
#include "Core/PolygonBuilder.h"

namespace Nork::Editor {
	class ColliderEditorPanel : public ViewportPanel
	{
	public:
		ColliderEditorPanel(Entity& ent);
		~ColliderEditorPanel();
		virtual void Content() override;
	private:
		PolygonBuilder polyBuilder;
		Entity& ent;
	};
}