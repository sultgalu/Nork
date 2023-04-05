#pragma once

#include "Panel.h"
#include "../../Views/include/Viewport.h"

namespace Nork::Editor {
	class ViewportPanel : public Panel
	{
	public:
		ViewportPanel();
		~ViewportPanel();
		virtual void Content() override;
		const char* GetName() override { return name.c_str(); };
		void OnContentSkipped() override;
		bool DeleteOnClose() const override { return true; }
		void InitializeWithRenderer();
	public:
		ViewportView viewportView;
	protected:
		std::string name = "Viewport";
	};
}