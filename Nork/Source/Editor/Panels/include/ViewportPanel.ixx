export module Nork.Editor.Panels:ViewportPanel;

import :Panel;
import Nork.Editor.Views;

export namespace Nork::Editor {
	class ViewportPanel : public Panel
	{
	public:
		ViewportPanel();
		~ViewportPanel();
		void Content() override;
		const char* GetName() override { return name.c_str(); };
		void SetIndex(int idx) { name = "Viewport  " + std::to_string(idx); }
		void OnContentSkipped() override;
		bool DeleteOnClose() const override { return true; }
	private:
		ViewportView viewportView;
		std::string name = "Viewport";
		std::shared_ptr<Viewport> viewport;
		std::shared_ptr<Components::Camera> camera;
	};
}