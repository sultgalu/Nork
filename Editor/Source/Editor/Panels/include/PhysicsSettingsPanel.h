#pragma once

#include "Panel.h"

namespace Nork::Editor {
	class PhysicsSettingsPanel : public Panel
	{
	public:
		PhysicsSettingsPanel();
		void Content() override;
		const char* GetName() override { return "Physics Settings"; };
	};
}