module Nork.Editor.Panels;

namespace Nork::Editor {
	ViewportPanel::ViewportPanel()
	{
		camera = std::make_shared<Components::Camera>();
		GetCommonData().editorCameras.push_back(camera);

		viewport = std::make_shared<Viewport>(camera);
		GetRenderer().viewports.push_back(viewport);

		viewportView.viewport = viewport;

		panelState.windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoScrollbar;
			//| ImGuiWindowFlags_NoSavedSettings;
	}
	ViewportPanel::~ViewportPanel()
	{
		for (size_t i = 0; i < GetRenderer().viewports.size(); i++)
		{
			if (GetRenderer().viewports[i] == viewport)
			{
				GetRenderer().viewports.erase(GetRenderer().viewports.begin() + i);
			}
		}
		for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
		{
			if (GetCommonData().editorCameras[i] == camera)
			{
				GetCommonData().editorCameras.erase(GetCommonData().editorCameras.begin() + i);
			}
		}
	}
	void ViewportPanel::Content()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Camera"))
			{
				for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
				{
					if (ImGui::Selectable(std::to_string(i).c_str()))
					{
						viewport->camera = GetCommonData().editorCameras[i];
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Phases"))
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
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Texture"))
			{
				if (ImGui::MenuItem("Default"))
				{
					viewport->target = nullptr;
				}
				if (ImGui::MenuItem("lightFb target"))
				{
					viewport->target = GetEngine().renderingSystem.deferredPipeline.lightFb->Color();
				}
				if (ImGui::MenuItem("GBuffer: depth"))
				{
					viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Depth();
				}
				if (ImGui::MenuItem("GBuffer: position"))
				{
					viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Position();
				}
				if (ImGui::MenuItem("GBuffer: normal"))
				{
					viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Normal();
				}
				if (ImGui::MenuItem("GBuffer: diffuse"))
				{
					viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Diffuse();
				}
				if (ImGui::MenuItem("GBuffer: specular"))
				{
					viewport->target = GetEngine().renderingSystem.deferredPipeline.geometryFb->Specular();
				}
				if (ImGui::MenuItem("Bool dest"))
				{
					viewport->target = GetEngine().renderingSystem.bloom.dest->GetAttachments()
						.colors[0].first;
				}
				ImGui::Separator();
				for (size_t i = 0; i < GetEngine().renderingSystem.bloom.fbs.size(); i++)
				{
					if (ImGui::MenuItem(("Bloom Texture Helper#" + std::to_string(i)).c_str()))
					{
						viewport->target = GetEngine().renderingSystem.bloom.fbs[i]->GetAttachments()
							.colors[0].first;
					}
				}
				ImGui::Separator();
				for (size_t i = 0; i < GetEngine().renderingSystem.bloom.fbs2.size(); i++)
				{
					if (ImGui::MenuItem(("Bloom2 Texture Helper#" + std::to_string(i)).c_str()))
					{
						viewport->target = GetEngine().renderingSystem.bloom.fbs2[i]->GetAttachments()
							.colors[0].first;
					}
				}
				ImGui::EndMenu();
			}
			ImGui::DragFloat("Cam Base Speed", &viewport->camera->moveSpeed, 0.001f);
			ImGui::Text(("texture ID: " + std::to_string(viewport->Texture()->GetHandle())).c_str());
			ImGui::EndMenuBar();
		}

		viewportView.viewport->active = true;
		viewportView.Content();

	}
	void ViewportPanel::OnContentSkipped()
	{
		viewportView.viewport->active = false;
	}
}
