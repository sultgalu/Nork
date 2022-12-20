#pragma once

#include "Core/Engine.h"
#include "Panels/include/Panel.h"
#include "Menus/include/Menu.h"
#include "Main/CommandBuffer.h"

namespace Nork::Editor {
	class Panel;
	class Menu;

	class Editor
	{
	public:
		Editor();
		static Editor& Get();
		void Render();
		void Update();
		void UpdateImguiInputs();
		void AddViewportPanel();
		void AddPanel(std::shared_ptr<Panel>);
		void RenderPassUI(CommandBuilder& builder);
		void createRenderPassUI();
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
	private:
		std::vector<std::shared_ptr<Panel>> panels;
		std::vector<std::shared_ptr<Menu>> menus;
		CommonData data;
		int viewportPanelCount = 0;
	public:
		VkDescriptorPool imguiPool;
		std::shared_ptr<RenderPass> renderPassUI;
		std::shared_ptr<Framebuffer> fbUI;
		std::shared_ptr<CommandPool> commandPool;
		VkDescriptorSet viewportImgDs;
		std::shared_ptr<ImageView> fbColor;
		std::shared_ptr<Sampler> textureSampler;
	};
}