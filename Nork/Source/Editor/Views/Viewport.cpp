#include "include/Viewport.h"

namespace Nork::Editor {
	static glm::vec2 GetDisplaySize(glm::vec2 texSize)
	{
		glm::vec2 displaySize = texSize;

		auto rMin = ImGui::GetWindowContentRegionMin();
		const static float ratio = ((float)texSize.x) / ((float)texSize.y);
		glm::vec2 panelSize = glm::vec2(ImGui::GetWindowSize().x - rMin.x, ImGui::GetWindowSize().y - rMin.y);
		float scale;
		if (panelSize.x / panelSize.y > ratio)
			scale = panelSize.y / (displaySize.y);
		else scale = panelSize.x / (displaySize.x);
		displaySize *= scale;

		return displaySize;
	}

	void ViewportView::Content()
	{
		auto texture = viewport->Texture();

		glm::vec2 texSize(texture->GetWidth(), texture->GetHeight());
		glm::vec2 displaySize = GetDisplaySize(texSize);

		constexpr auto uv_min = glm::vec2(0, 1);
		constexpr auto uv_max = glm::vec2(1, 0);
		ImGui::Image((ImTextureID)texture->GetHandle(), ImVec2(displaySize.x, displaySize.y), ImVec2(uv_min.x, uv_min.y),
			ImVec2(uv_max.x, uv_max.y), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
		if (ImGui::IsItemHovered())
		{
			CameraController::UpdateByKeyInput(*viewport->camera, ImGui::GetIO().DeltaTime * 500);
			CameraController::UpdateByMouseInput(*viewport->camera, ImGui::GetIO().DeltaTime * 500);
		}

		auto rMin = ImGui::GetWindowContentRegionMin();
		auto wPos = ImGui::GetWindowPos();
		auto mPos = ImGui::GetMousePos();
		glm::vec2 actualPos = glm::vec2(mPos.x - wPos.x - rMin.x, displaySize.y - (mPos.y - wPos.y - rMin.y));
		glm::vec2 actualPos2 = glm::vec2((actualPos.x / displaySize.x) * texSize.x, (actualPos.y / displaySize.y) * texSize.y);
	}
	ViewportView::ViewportView()
	{}
}