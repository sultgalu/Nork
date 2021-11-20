#include "pch.h"
#include "Panel.h"
#include "App/Application.h"

namespace Nork::Editor
{
	Panel::Panel(std::string name, EditorData& data)
		: data(data), name(name),
		events(Application::Get().dispatcher.GetReceiver().Map<Event::Types::Base>([&]() { return this->state.isHovered; }))
	{
	}
	void Panel::Draw()
	{
		// can set stuff based on global / local settings (color, font etc...)
		Begin();
		UpdateState();
		DrawContent();
		End();
	}
	void Panel::UpdateState()
	{
		this->state.isFocused = ImGui::IsWindowFocused();
		this->state.isCollapsed = ImGui::IsWindowCollapsed();
		this->state.isDocked = ImGui::IsWindowDocked();
		this->state.isHovered = ImGui::IsWindowHovered();
		this->state.isAppearing = ImGui::IsWindowAppearing();
	}
	void Panel::Begin()
	{
		ImGui::Begin(this->GetName().c_str());
	}
	void Panel::End()
	{
		ImGui::End();
	}
}