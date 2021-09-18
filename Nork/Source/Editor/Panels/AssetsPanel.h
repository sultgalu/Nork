#pragma once

#include "Base/Panel.h"

namespace Nork::Editor
{
	class AssetsPanel: public Panel
	{
	public:
		AssetsPanel() : Panel("Assets", true) {}
	protected:
		virtual void DrawContent() override;
	};
}

