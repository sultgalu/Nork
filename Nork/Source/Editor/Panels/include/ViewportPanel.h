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
		void SetIndex(int idx) { name = "Viewport  " + std::to_string(idx); }
		void OnContentSkipped() override;
		bool DeleteOnClose() const override { return true; }
	protected:
		ViewportView viewportView;
		std::string name = "Viewport";
		std::shared_ptr<SceneView> sceneView;
		std::shared_ptr<Components::Camera> camera;
	};
}