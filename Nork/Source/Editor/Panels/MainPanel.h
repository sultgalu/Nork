#pragma once

#include "Base/Panel.h"

namespace Nork::Editor
{
	class MainPanel : public Panel
	{
	public:
		MainPanel() : Panel("MainPanel", true) {}
	protected:
		virtual void DrawContent() override;
	};
}

