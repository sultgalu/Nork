#pragma once

#include "Core/Engine.h"
#include "Editor2/EditorData.h"

namespace Nork::Editor2
{
	struct PanelState
	{
		bool isFocused, isHovered, isDocked, isAppearing, isCollapsed;
	};

	class Panel
	{
	public:
		Panel(std::string name, EditorData& data);
		void Draw();
	protected:
		virtual void DrawContent() {}; // Draws the content of the panel
		virtual void UpdateState(); // Updates state member
		virtual void Begin(); // begin panel (should call ImGui::Begin())
		virtual void End(); // end the panel (should call ImGui::End())

		inline const std::string& GetName() { return name; }
		inline const PanelState& GetState() { return state; }
	protected:
		PanelState state;
		EditorData& data;
		std::string name;
		//Receiver events; // = reg.GetReceiver();
	};
}

