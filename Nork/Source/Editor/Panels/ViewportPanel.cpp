#include "pch.h"
#include "ViewportPanel.h"

int viewportCounter = 0;
Nork::Editor::ViewportPanel::ViewportPanel() 
	: Nork::Editor::Panel(std::format("Viewport#{}", viewportCounter++).c_str(), true),
	mouseState(MouseState{}), image(ImageConfig{})
{
}

Nork::Editor::ViewportPanel::~ViewportPanel()
{
}

glm::vec2 GetDisplaySize(glm::vec2 texSize)
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

void Nork::Editor::ViewportPanel::DrawContent()
{
	ImGui::Begin(this->GetName().data(), 0, ImGuiWindowFlags_NoScrollbar);

	glm::vec2 texSize(image.resolution.x, image.resolution.y);
	glm::vec2 displaySize = GetDisplaySize(texSize);

	ImGui::Image((ImTextureID)image.texture, ImVec2(displaySize.x, displaySize.y), ImVec2(image.uv_min.x, image.uv_min.y),
		ImVec2(image.uv_max.x, image.uv_max.y), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
	mouseState.isViewportHovered = ImGui::IsItemHovered();
	mouseState.isViewportDoubleClicked = mouseState.isViewportHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

	auto rMin = ImGui::GetWindowContentRegionMin();
	auto wPos = ImGui::GetWindowPos();
	auto mPos = ImGui::GetMousePos();
	glm::vec2 actualPos = glm::vec2(mPos.x - wPos.x - rMin.x, displaySize.y - (mPos.y - wPos.y - rMin.y));
	glm::vec2 actualPos2 = glm::vec2((actualPos.x / displaySize.x) * texSize.x, (actualPos.y / displaySize.y) * texSize.y);
	mouseState.mousePosX = (int)actualPos2.x;
	mouseState.mousePosY = (int)actualPos2.y;

	if (ImGui::Button("Chosse texture to display"))
		ImGui::OpenPopup("texturesToDisplay");
	ImGui::SameLine();
	ImGui::Text((std::to_string(actualPos2.x) + ";" + std::to_string(actualPos2.y)).c_str());

	static const char* const names[]
	{
		"Final",
		"GPosition", "GDiffuse", "GNormal", "GDepth", "DeferredResult", "GOutl",
		"DirShadow", "LightPass", "Bloom" , "SSAO", "SSAONoise", "dShadow0", "dShadow1", "dShadow2"
	};
	int channels[] = { 3, 3, 3, 3, 1, 1, 3, 3, 1, 3 };
	static int selected = 0;

	//if (ImGui::BeginPopup("texturesToDisplay"))
	//{
	//	for (int i = 0; i < sizeof(names) / sizeof(char*); i++)
	//	{
	//		if (ImGui::Selectable(names[i]))
	//		{
	//			selected = i;
	//			//this->data.tex = scene.pipeline.GetTexture(textures[selected]);
	//			if (selected == 0)
	//				this->data.tex = scene.engine.GetPostProcessor().data.final.tex;
	//			if (selected == 1)
	//				this->data.tex = scene.engine.GetDefPipeline().data.gBuffer.pos;
	//			if (selected == 2)
	//				this->data.tex = scene.engine.GetDefPipeline().data.gBuffer.diff;
	//			if (selected == 3)
	//				this->data.tex = scene.engine.GetDefPipeline().data.gBuffer.norm;
	//			if (selected == 4)
	//				this->data.tex = scene.engine.GetDefPipeline().data.gBuffer.depth;
	//			if (selected == 5)
	//				this->data.tex = scene.engine.GetDefPipeline().data.lightPass.tex;
	//		}
	//		if (i == selected)
	//			ImGui::SetItemDefaultFocus();
	//	}
	//	ImGui::EndPopup();
	//}

	ImGui::End();
}
