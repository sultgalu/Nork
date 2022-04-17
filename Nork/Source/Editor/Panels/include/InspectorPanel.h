#pragma once
#include "Panel.h"

namespace Nork::Editor {

	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel();
		void Content() override;
		const char* GetName() override { return "Inspector"; };
	private:
	};
}