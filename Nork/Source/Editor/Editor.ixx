export module Nork.Editor;

import Nork.Core;
import Nork.Editor.Panels;
import Nork.Editor.Menus;

export namespace Nork::Editor {
	class Panel;
	class Menu;

	class Editor
	{
	public:
		Editor(Engine& engine);
		void Render();
		void Update();
		void UpdateImguiInputs();
		void AddViewportPanel(); 
		static Editor& Get();
	private:
		void DrawPanelManager();
		template<std::derived_from<Panel> T>
		std::shared_ptr<T> GetPanel()
		{
			for (auto& panel : panels)
			{
				if (dynamic_cast<T*>(panel.get()))
				{
					return panel;
				}
			}
			return nullptr;
		}
	public:
		CommonData data;
		Engine& engine;
	private:
		std::vector<std::unique_ptr<Panel>> panels;
		std::vector<std::unique_ptr<ShadersPanel>> shaderPanels;
		std::vector<std::unique_ptr<Menu>> menus;
		int viewportPanelCount = 0;
	};
}