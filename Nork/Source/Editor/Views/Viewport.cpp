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

		if (ImGui::BeginPopup("texturesToDisplay"))
		{
			if (ImGui::Selectable("Default"))
			{
				viewport->target = nullptr;
			}
			if (ImGui::Selectable("lightFb target"))
			{
				viewport->target = GetEngine().renderingSystem.deferredPipeline.lightFb->Color();
			}
			if (ImGui::Selectable("GBuffer: depth"))
			{
				viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Depth();
			}
			if (ImGui::Selectable("GBuffer: position"))
			{
				viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Position();
			}
			if (ImGui::Selectable("GBuffer: normal"))
			{
				viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Normal();
			}
			if (ImGui::Selectable("GBuffer: diffuse"))
			{
				viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Diffuse();
			}
			if (ImGui::Selectable("GBuffer: specular"))
			{
				viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Specular();
			}
			if (ImGui::Selectable("Bool dest"))
			{
				viewport->target = GetEngine().renderingSystem.bloom.dest->GetAttachments()
					.colors[0].first;
			}
			for (size_t i = 0; i < GetEngine().renderingSystem.bloom.fbs.size(); i++)
			{
				if (ImGui::Selectable(("Bloom Texture Helper#" + std::to_string(i)).c_str()))
				{
					viewport->target = GetEngine().renderingSystem.bloom.fbs[i]->GetAttachments()
						.colors[0].first;
				}
			}
			for (size_t i = 0; i < GetEngine().renderingSystem.bloom.fbs2.size(); i++)
			{
				if (ImGui::Selectable(("Bloom2 Texture Helper#" + std::to_string(i)).c_str()))
				{
					viewport->target = GetEngine().renderingSystem.bloom.fbs2[i]->GetAttachments()
						.colors[0].first;
				}
			}

			ImGui::EndPopup();
		}

		if (ImGui::Button("Choose texture to display"))
			ImGui::OpenPopup("texturesToDisplay");
		ImGui::SameLine();
		// ImGui::Text((std::to_string(actualPos2.x) + ";" + std::to_string(actualPos2.y)).c_str());
		ImGui::Text(("texture ID: " + std::to_string(texture->GetHandle())).c_str());
		ImGui::SameLine();
		if (ImGui::Button("Camera"))
			ImGui::OpenPopup("chooseCam");
		ImGui::SameLine();
		if (ImGui::Button("Phases"))
			ImGui::OpenPopup("choosePhases");

		if (ImGui::BeginPopup("chooseCam"))
		{
			for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
			{
				if (ImGui::Selectable(std::to_string(i).c_str()))
				{
					viewport->camera = GetCommonData().editorCameras[i];
				}
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("choosePhases"))
		{
			auto checkbox = [&](Viewport::Source source, const char* label)
			{
				bool enabled = (viewport->source & source) == source;
				if (ImGui::Checkbox(label, &enabled))
				{
					if (enabled)
						viewport->source |= source;
					else
						viewport->source &= ~source;
				}
			};
			checkbox(Viewport::Source::Deferred, "Deferred");
			checkbox(Viewport::Source::Bloom, "Bloom");
			checkbox(Viewport::Source::Sky, "Sky");
			checkbox(Viewport::Source::Tonemap, "Tonemap");
			checkbox(Viewport::Source::Colliders, "Colliders");
			ImGui::Separator();
			checkbox(Viewport::Source::Default, "Default");
			checkbox(Viewport::Source::PostProcess, "PostProcess");
			checkbox(Viewport::Source::Debug, "Debug");
			ImGui::EndPopup();
		}
	}
	ViewportView::ViewportView()
	{}
}