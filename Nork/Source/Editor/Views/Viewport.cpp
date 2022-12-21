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
		if (ImGui::IsItemHovered())
		{
			// camController->UpdateByKeyInput(*sceneView->camera, ImGui::GetIO().DeltaTime * 500);
			// camController->UpdateByMouseInput(*sceneView->camera);
		}
		if (viewportImgDs != VK_NULL_HANDLE)
		{
			glm::vec2 texSize(image->Image()->Width(), image->Image()->Height());
			ImVec2 displaySize = ImGui::GetContentRegionAvail(); //GetDisplaySize(texSize);

			constexpr auto uv_min = glm::vec2(0, 1);
			constexpr auto uv_max = glm::vec2(1, 0);
			ImGui::Image(viewportImgDs, ImVec2(displaySize.x, displaySize.y));
		}
		     
		mouseState.isViewportHovered = ImGui::IsItemHovered();
		mouseState.isViewportDoubleClicked = mouseState.isViewportHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		mouseState.isViewportClicked = mouseState.isViewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

		auto rMin = ImGui::GetWindowContentRegionMin();
		auto wPos = ImGui::GetWindowPos();
		auto mPos = ImGui::GetMousePos();
		// glm::vec2 actualPos = glm::vec2(mPos.x - wPos.x - rMin.x, displaySize.y - (mPos.y - wPos.y - rMin.y));
		// glm::vec2 actualPos2 = glm::vec2((actualPos.x / displaySize.x) * texSize.x, (actualPos.y / displaySize.y) * texSize.y);
		// mouseState.mousePosX = (int)actualPos2.x;
		// mouseState.mousePosY = (int)actualPos2.y;
	}
	ViewportView::ViewportView()
	{}
}