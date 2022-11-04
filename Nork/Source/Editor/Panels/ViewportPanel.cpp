#include "include/ViewportPanel.h"
#include "Modules/Renderer/Pipeline/Stages/BloomStage.h"
#include "Modules/Renderer/Pipeline/Stages/SkyStage.h"
#include "Modules/Renderer/Pipeline/Stages/SkyboxStage.h"
#include "Modules/Renderer/Pipeline/Stages/PostProcessStage.h"

namespace Nork::Editor {
	ViewportPanel::ViewportPanel(bool autoStages)
	{
		sceneView = std::make_shared<SceneView>(1920, 1080, Renderer::TextureFormat::RGBA16F);
		if (autoStages)
		{
			sceneView->pipeline->stages.push_back(GetRenderer().CreateStage<Renderer::DeferredStage>(*sceneView->pipeline));
			//sceneView->pipeline->stages.push_back(GetRenderer().CreateStage<Renderer::SkyStage>(*sceneView->pipeline));
			// sceneView->pipeline->stages.push_back(GetRenderer().CreateStage<Renderer::BloomStage>(*sceneView->pipeline));
			//sceneView->pipeline->stages.push_back(GetRenderer().CreateStage<Renderer::PostProcessStage>(*sceneView->pipeline));
			// sceneView->pipeline->stages.push_back(std::make_shared<CollidersStage>(GetScene(), GetRenderer().shaders));
		}
		GetRenderer().sceneViews.insert(sceneView);
		camera = sceneView->camera;
		GetCommonData().editorCameras.push_back(camera);

		viewportView.sceneView = sceneView;

		panelState.windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoScrollbar;
			//| ImGuiWindowFlags_NoSavedSettings;
	}
	ViewportPanel::~ViewportPanel()
	{
		GetRenderer().sceneViews.erase(sceneView);
		for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
		{
			if (GetCommonData().editorCameras[i] == camera)
			{
				GetCommonData().editorCameras.erase(GetCommonData().editorCameras.begin() + i);
			}
		}
	}
	static const char* NameForStage(const type_info& type)
	{
		if (typeid(Renderer::DeferredStage) == type) return "Deferred";
		if (typeid(Renderer::BloomStage) == type) return "Bloom";
		if (typeid(Renderer::SkyStage) == type) return "Sky";
		if (typeid(Renderer::SkyboxStage) == type) return "Skybox";
		if (typeid(Renderer::PostProcessStage) == type) return "Post Process";
		return "NOT_FOUND";
	}
	static const char* NameForStage(const Renderer::Stage* stage)
	{
		return NameForStage(typeid(*stage));
	}
	template<std::derived_from<Renderer::Stage> T> static const char* NameForStage()
	{
		return NameForStage(typeid(T));
	}
	template<std::derived_from<Renderer::Stage> T> static void AddStageItem(RenderingSystem& renderingSystem, Renderer::Pipeline& pipeline)
	{
		auto name = NameForStage(typeid(T));
		if (ImGui::Selectable((name + std::string("##stage")).c_str(), false, ImGuiSelectableFlags_DontClosePopups))
		{
			pipeline.stages.push_back(renderingSystem.CreateStage<T>(pipeline));
		}
	}
	void ViewportPanel::Content()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Camera"))
			{
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
						sceneView->camera = GetCommonData().editorCameras[i];
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Stages"))
			{
				for (auto it = sceneView->pipeline->stages.begin(); it != sceneView->pipeline->stages.end(); it++)
				{
					auto id = NameForStage((*it).get()) + std::string("#") + std::to_string((int)(it._Ptr));
					if (ImGui::SmallButton(("X##" + id).c_str()))
					{
						auto toRemove = it--;
						sceneView->pipeline->stages.erase(toRemove);
						continue;
					}
					ImGui::SameLine();
					ImGui::Selectable(id.c_str(), false, ImGuiSelectableFlags_DontClosePopups);
					if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
					{
						bool up = ImGui::GetMouseDragDelta(0).y < 0.0f;
						if ((up && it != sceneView->pipeline->stages.begin()) || !up && it != --sceneView->pipeline->stages.end())
						{
							auto itOther = it;
							if (up)
								itOther--;
							else
							{
								++(++itOther);
							}
							sceneView->pipeline->stages.splice(itOther, sceneView->pipeline->stages, it);
							ImGui::ResetMouseDragDelta();
						}
					}
				}
				ImGui::Separator();
				if (ImGui::BeginMenu("Add"))
				{
					AddStageItem<Renderer::DeferredStage>(GetRenderer(), *sceneView->pipeline);
					AddStageItem<Renderer::BloomStage>(GetRenderer(), *sceneView->pipeline);
					AddStageItem<Renderer::SkyStage>(GetRenderer(), *sceneView->pipeline);
					AddStageItem<Renderer::SkyboxStage>(GetRenderer(), *sceneView->pipeline);
					AddStageItem<Renderer::PostProcessStage>(GetRenderer(), *sceneView->pipeline);
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
			ImGui::DragFloat("Cam Base Speed", &sceneView->camera->moveSpeed, 0.001f);
			ImGui::Text(("texture ID: " + std::to_string(sceneView->fb->Color()->GetHandle())).c_str());
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
