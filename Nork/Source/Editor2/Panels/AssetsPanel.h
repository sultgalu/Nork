#pragma once

#include "Base/Panel.h"

namespace Nork::Editor2
{
	class AssetsPanel: public Panel
	{
	public:
		AssetsPanel(EditorData& data) : Panel("Assets", data) {}
	protected:
		virtual void DrawContent() override;
	};
}

