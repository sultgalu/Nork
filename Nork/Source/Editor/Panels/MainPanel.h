#pragma once
#include "Base/Panel.h"

namespace Nork::Editor
{
	class MainPanel : public Panel
	{
	public:
		MainPanel(EditorData& d) : Panel("MainPanel", d) {}
	protected:
		virtual void Begin() override;
		virtual void End() override;
		virtual void DrawContent() override;
	};
}

