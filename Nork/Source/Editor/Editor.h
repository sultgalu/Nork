#pragma once

#include "Core/Engine.h"
#include "Panels/include/Panel.h"
#include "Menus/include/Menu.h"
#include "Modules/Renderer/Vulkan/CommandBuffer.h"

namespace Nork::Editor {
	class Panel;
	class Menu;

	class Editor
	{
	public:
		Editor();
		~Editor();
		static Editor& Get();
		void Render();
		void Update();
		void UpdateImguiInputs();
		void AddViewportPanel();
		void AddPanel(std::shared_ptr<Panel>);
		
		void InitImguiForVulkanAndGlfw();
	private:
		void DrawPanelManager();
		template<std::derived_from<Panel> T> std::shared_ptr<T> GetPanel()
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
		std::vector<std::shared_ptr<Panel>> panels;
		std::vector<std::shared_ptr<Menu>> menus;
		CommonData data;
		int viewportPanelCount = 0;
	public:
	};
}