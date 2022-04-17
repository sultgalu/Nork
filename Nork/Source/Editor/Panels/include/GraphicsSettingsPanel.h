#pragma once
#include "Panel.h"

namespace Nork::Editor {

	class GraphicsSettingsPanel : public Panel
	{
	public:
		GraphicsSettingsPanel();
		void Content() override;
		const char* GetName() override { return "Graphics Settings"; };
	};
}