#pragma once

#include "Base/Panel.h"

namespace Nork::Editor
{
	class AssetsPanel: public Panel
	{
	public:
		AssetsPanel() : Panel(std::string("Assets").c_str(), true) {}
	protected:
		virtual void DrawContent() override;
	};
}
