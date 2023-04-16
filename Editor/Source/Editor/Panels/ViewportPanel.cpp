#include "include/ViewportPanel.h"

namespace Nork::Editor {
	ViewportPanel::ViewportPanel()
	{
		panelState.windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoScrollbar;
			//| ImGuiWindowFlags_NoSavedSettings;
	}
	void ViewportPanel::InitializeWithRenderer()
	{
		RenderingSystem::Instance().camera = viewportView.camera;
		viewportView.image = Renderer::Renderer::Instance().mainImage;
	}
	struct ViewportInit {
		ViewportInit(ViewportPanel& vp) {
			vp.InitializeWithRenderer();
		}
	};
	ViewportPanel::~ViewportPanel()
	{
	}
	void ViewportPanel::Content()
	{
		static ViewportInit vpInit(*this); // needed because of bad Editor/Engine init order https://github.com/sultgalu/Nork/issues/7
		if (auto editorController = std::dynamic_pointer_cast<EditorCameraController>(viewportView.camController)) {
			if (GetCommonData().selectedNode) {
				if (Input::Instance().IsJustPressed(Key::F)) {
					if (focusedNode.lock()) { // if already focusing on smt, stop it
						focusedNode.reset();
					}
					else if (auto tr = GetCommonData().selectedNode->GetEntity().TryGetComponent<Components::Transform>()) {
						focusedNode = GetCommonData().selectedNode;
					}
				}
			}
			if (Input::Instance().IsJustPressed(Key::Z)) {
				editorController->SetDistance(*viewportView.camera, 10);
			}

			if (auto node = focusedNode.lock()) { 
				if (auto tr = node->GetEntity().TryGetComponent<Components::Transform>()) {
					editorController->SetCenter(*viewportView.camera, tr->position);
				}
			}
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Camera"))
			{
				auto camera = viewportView.camera;
				if (ImGui::BeginMenu("Behaviour"))
				{
					auto fpsController = std::dynamic_pointer_cast<FpsCameraController>(viewportView.camController);
					if (ImGui::MenuItem("FPS", 0, fpsController != nullptr)) {
						fpsController = std::make_shared<FpsCameraController>();
						viewportView.camController = fpsController;
					}
					auto editorController = std::dynamic_pointer_cast<EditorCameraController>(viewportView.camController);
					if (ImGui::MenuItem("Editor", 0, editorController != nullptr)) {
						editorController = std::make_shared<EditorCameraController>(glm::vec3(0, 0, 0));
						viewportView.camController = editorController;
					}
					if (editorController) {
						if (ImGui::DragFloat3("Center##editorController", &editorController->center.x, 0.1f)){
							editorController->UpdateByMouseInput(*camera);
						}
						ImGui::DragFloat("Zoom Speed", &editorController->zoomSpeed, 0.01f, 0.001f);
						ImGui::DragFloat("Rotation Speed", &editorController->rotationSpeed, 0.01f, 0.001f);
					}
					else if (fpsController) {
						ImGui::DragFloat("Movement Speed", &fpsController->moveSpeed, 0.01f, 0.001f);
						ImGui::DragFloat("Zoom Speed", &fpsController->zoomSpeed, 0.01f, 0.001f);
						ImGui::DragFloat("Rotation Speed", &fpsController->rotationSpeed, 0.01f, 0.001f);
					}
					ImGui::EndMenu();
				}
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
