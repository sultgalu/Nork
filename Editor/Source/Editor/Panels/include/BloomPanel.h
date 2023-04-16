#pragma once
#include "Panel.h"

namespace Nork::Editor {

	class BloomPanel : public Panel
	{
	public:
		BloomPanel();
		void Content() override;
		const char* GetName() override { return "Bloom"; };
	};
}