#include "include/Viewport.h"

namespace Nork::Editor {
	static ImVec2 GetDisplaySize(glm::vec2 texSize)
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

		return ImVec2(displaySize.x, displaySize.y);
	}

	void ViewportView::Content()
	{
		if (image)
		{
			glm::vec2 texSize(image.Img().img->Width(), image.Img().img->Height());
			ImVec2 displaySize = GetDisplaySize(texSize); // ImGui::GetContentRegionAvail();

			constexpr auto uv_min = glm::vec2(0, 1);
			constexpr auto uv_max = glm::vec2(1, 0);
			ImGui::Image(image.descritptorSet, ImVec2(displaySize.x, displaySize.y));
			if (ImGui::IsItemHovered())
			{
				camController->UpdateByKeyInput(*camera, ImGui::GetIO().DeltaTime * 500);
				camController->UpdateByMouseInput(*camera);
			}
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
	{
		camera = std::make_shared<Components::Camera>();
		camera->position = glm::vec3(20);
		camController = std::make_shared<EditorCameraController>(glm::vec3(0));
	}
}