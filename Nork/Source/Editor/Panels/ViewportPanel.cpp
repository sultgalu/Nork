#include "include/ViewportPanel.h"

namespace Nork::Editor {
	ViewportPanel::ViewportPanel()
	{
		GetCommonData().editorCameras.push_back(viewportView.camera);
		RenderingSystem::Instance().camera = viewportView.camera;

		panelState.windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoScrollbar;
			//| ImGuiWindowFlags_NoSavedSettings;
		if (Renderer::Renderer::Instance().instance)
		{
			auto& renderPasses = Renderer::Renderer::Instance().renderPasses;
			for (auto& pass : renderPasses)
			{
				if (auto dp = std::dynamic_pointer_cast<Renderer::DeferredPass>(pass))
				{
					viewportView.image = dp->fbColor;
					return;
				}
			}
			std::unreachable(); // could not find displayable img
		}
	}
	ViewportPanel::~ViewportPanel()
	{
		for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
		{
			if (GetCommonData().editorCameras[i] == viewportView.camera)
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
				auto camera = viewportView.camera;
				if (ImGui::BeginMenu("Behaviour"))
				{
					if (ImGui::MenuItem("FPS", 0, std::dynamic_pointer_cast<FpsCameraController>(viewportView.camController) != nullptr))
						viewportView.camController = std::make_shared<FpsCameraController>();
					auto editorController = std::dynamic_pointer_cast<EditorCameraController>(viewportView.camController);
					if (ImGui::MenuItem("Editor", 0, editorController != nullptr))
						viewportView.camController = std::make_shared<EditorCameraController>(glm::vec3(0, 0, 0));
					if (editorController != nullptr)
						if (ImGui::DragFloat3("Center##editorController", &editorController->center.x, 0.1f))
						{
							editorController->UpdateByMouseInput(*camera);
						}
					ImGui::EndMenu();
				}
				ImGui::DragFloat("Movement Speed", &camera->moveSpeed, 0.01f, 0.001f);
				ImGui::DragFloat("Zoom Speed", &camera->zoomSpeed, 0.01f, 0.001f);
				ImGui::DragFloat("Rotation Speed", &camera->rotationSpeed, 0.01f, 0.001f);
				auto fov = glm::degrees(camera->FOV);
				if (ImGui::DragFloat("FOV", &fov, 0.01f, 0.00f, 180.0f))
				{
					camera->FOV = glm::radians(fov);
					camera->Update();
				}
				// ImGui::DragFloat("Near clip", &camera->nearClip, 0.01f, 0, 0, "%.3f", ImGuiSliderFlags_Logarithmic);
				// ImGui::DragFloat("Far clip", &camera->farClip, 1.0f, 0, 0, "%.3f", ImGuiSliderFlags_Logarithmic);
				if (ImGui::BeginMenu("Aspect Ratio"))
				{
					auto choice = [&](int w, int h)
					{
						float val = w / (float)h;
						if (ImGui::MenuItem(std::to_string(w).append(":").append(std::to_string(h)).c_str(), 0, camera->ratio == val))
						{
							camera->ratio = val;
							camera->Update();
						}
					};
					choice(16, 9);
					choice(4, 3);
					choice(1, 1);
					ImGui::EndMenu();
				}
				for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
				{
					if (ImGui::Selectable(std::to_string(i).c_str()))
					{
						camera = GetCommonData().editorCameras[i];
					}
				}
				ImGui::EndMenu();
			}
			/*if (ImGui::BeginMenu("Texture"))
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
			}*/
			ImGui::DragFloat("Cam Base Speed", &viewportView.camera->moveSpeed, 0.001f);
			ImGui::EndMenuBar();
		}

		//viewportView.viewport->active = true;
		viewportView.Content();

	}
	void ViewportPanel::OnContentSkipped()
	{
		// viewportView.viewport->active = false;
	}
}
