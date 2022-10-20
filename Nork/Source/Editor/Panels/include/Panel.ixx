export module Nork.Editor.Panels:Panel;

import Nork.Editor.Views;

export namespace Nork::Editor {
	struct PanelState
	{
		bool isFocused, isHovered, isDocked, isAppearing, isCollapsed;
		bool isOpen = true;
		bool setFocus = false;
		ImGuiWindowFlags windowFlags = 0;
	};

	class Panel: public View
	{
	public:
		bool Draw()
		{
			if (panelState.setFocus)
			{
				ImGui::SetNextWindowFocus();
				panelState.setFocus = false;
			}
			if (panelState.isOpen)
			{
				if (ImGui::Begin(GetName(), &panelState.isOpen, panelState.windowFlags))
				{
					panelState.isFocused = ImGui::IsWindowFocused();
					panelState.isCollapsed = ImGui::IsWindowCollapsed();
					panelState.isDocked = ImGui::IsWindowDocked();
					panelState.isHovered = ImGui::IsWindowHovered();
					panelState.isAppearing = ImGui::IsWindowAppearing();

					if (!panelState.isCollapsed)
					{
						Content();
					}
					else
					{
						OnContentSkipped();
					}
				}
				else
				{
					OnContentSkipped();
				}
				ImGui::End();
				return true;
			}
			else
			{
				return false;
			}
		}
		virtual void OnContentSkipped() {};
		virtual const char* GetName() = 0;
		virtual bool DeleteOnClose() const { return false; }
	public:
		PanelState panelState;
	};
}