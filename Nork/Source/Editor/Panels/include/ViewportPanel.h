#pragma once

#include "Panel.h"
#include "../../Views/include/Viewport.h"

namespace Nork::Editor {
	class ViewportPanel : public Panel
	{
	public:
		ViewportPanel();
		~ViewportPanel();
		void Content() override;
		const char* GetName() override { return name.c_str(); };
		void SetIndex(int idx) { name = "Viewport  " + std::to_string(idx); }
		void OnContentSkipped() override;
	private:
		ViewportView viewportView;
		std::string name = "Viewport";
		std::shared_ptr<Viewport> viewport;
		std::shared_ptr<Components::Camera> camera;
	};
}