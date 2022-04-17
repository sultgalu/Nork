#include "include/Helpers.h"

namespace Nork::Editor::Helpers {
	bool TextEditable(std::string& text)
	{
		ImGui::Text(text.c_str());

		static constexpr size_t bufSize = 100;
		static char buf[bufSize];
		ImGui::SameLine();
		if (ImGui::Button(("Change##" + text).c_str()))
		{
			std::memcpy(buf, text.c_str(), text.size());
			std::memset(&buf[text.size()], 0, bufSize - text.size());
			ImGui::OpenPopup(("pupup##" + text).c_str());
		}
		bool changed = false;
		if (ImGui::BeginPopup(("pupup##" + text).c_str()))
		{
			if (ImGui::InputText("", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				text = std::string(buf);
				changed = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return changed;
	}
}