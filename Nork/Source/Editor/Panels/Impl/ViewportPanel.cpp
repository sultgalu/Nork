#include "pch.h"
#include "../ViewportPanel.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"
#include "App/Application.h"

namespace Nork::Editor
{
	static int viewportCounter = 0;
	static std::shared_ptr<Renderer::Texture2D> defaultTex;

	ViewportPanel::ViewportPanel(EditorData& d)
		: Nork::Editor::Panel(std::format("Viewport#{}", viewportCounter++), d),
		mouseState(MouseState{}), image(ImageConfig{})
	{
		uint8_t texData[3] = { 128, 228 , 0 }; 
		defaultTex = Renderer::TextureBuilder()
			.Params(TextureParams::Tex2DParams())
			.Attributes(Renderer::TextureAttributes{ .width = 1, .height = 1, .format = Renderer::TextureFormat::RGB })
			.Create2DWithData(texData);
		image.texture = defaultTex;
		data.engine.AddCamera(cam1);
		data.engine.AddCamera(cam2);
	}

	ViewportPanel::~ViewportPanel()
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

	void ViewportPanel::Begin()
	{
		data.engine.Cameras()[0] = cam1;
		data.engine.Cameras()[1] = cam2;
		// camContr.UpdateByKeyInput(data.engine.scene.GetMainCamera(), ImGui::GetIO().DeltaTime * 500);
		// if (state.isHovered)
		// 	camContr.UpdateByMouseInput(data.engine.scene.GetMainCamera(), ImGui::GetIO().DeltaTime * 500);
		ImGui::Begin(this->GetName().data(), 0, ImGuiWindowFlags_NoScrollbar);
	}

	void ViewportPanel::DrawContent()
	{
		data.idQueryMode.reset();

		glm::vec2 texSize(image.resolution.x, image.resolution.y);
		glm::vec2 displaySize = GetDisplaySize(texSize);

		if (ImGui::BeginTabBar("texes"))
		{
			if (ImGui::BeginTabItem("tex1"))
			{
				ImGui::Image((ImTextureID)image.texture->GetHandle(), ImVec2(displaySize.x, displaySize.y), ImVec2(image.uv_min.x, image.uv_min.y),
					ImVec2(image.uv_max.x, image.uv_max.y), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
				if (ImGui::IsItemHovered())
				{
					camContr.UpdateByKeyInput(cam1, ImGui::GetIO().DeltaTime * 500);
					if (state.isHovered)
						camContr.UpdateByMouseInput(cam1, ImGui::GetIO().DeltaTime * 500);
					mouseState.isViewportHovered = ImGui::IsItemHovered();
					mouseState.isViewportDoubleClicked = mouseState.isViewportHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("tex2"))
			{
				ImGui::Image((ImTextureID)data.engine.renderingSystem.GetTargetFramebuffers()[1]->GetAttachments().colors[0].first->GetHandle(), ImVec2(displaySize.x, displaySize.y), ImVec2(image.uv_min.x, image.uv_min.y),
					ImVec2(image.uv_max.x, image.uv_max.y), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
				if (ImGui::IsItemHovered())
				{
					camContr.UpdateByKeyInput(cam2, ImGui::GetIO().DeltaTime * 500);
					if (state.isHovered)
						camContr.UpdateByMouseInput(cam2, ImGui::GetIO().DeltaTime * 500);
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

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
		ImGui::Text(("texture ID: " + std::to_string(image.texture->GetHandle())).c_str());

		static glm::vec2 res;
		ImGui::SliderFloat2("asd", &res.x, 0, 2000);
		if (ImGui::Button("Change Res"))
		{
			auto& pipeline = data.engine.renderingSystem.deferredPipeline;
			pipeline = Renderer::DeferredPipeline(pipeline.geomatryShader, 
				pipeline.lightShader, res.x, res.y);
			image.texture = pipeline.lightFb->Color();
		}

		if (ImGui::BeginPopup("texturesToDisplay"))
		{
			if (ImGui::Selectable("Default"))
			{
				image.texture = data.engine.renderingSystem.GetTargetFramebuffers()[0]->GetAttachments().colors[0].first;
			}
			if (ImGui::Selectable("lightFb target"))
			{
				image.texture = data.engine.renderingSystem.deferredPipeline.lightFb->Color();
			}
			/*if (ImGui::Selectable("Ids"))
			{
				image.texture = data.engine.idMap;
			}*/
			if (ImGui::Selectable("GBuffer: depth"))
			{
				image.texture = data.engine.renderingSystem.deferredPipeline.geometryFb->Depth();
			}
			if (ImGui::Selectable("GBuffer: position"))
			{
				image.texture = data.engine.renderingSystem.deferredPipeline.geometryFb->Position();
			}
			if (ImGui::Selectable("GBuffer: normal"))
			{
				image.texture = data.engine.renderingSystem.deferredPipeline.geometryFb->Normal();
			}
			if (ImGui::Selectable("GBuffer: diffuse"))
			{
				image.texture = data.engine.renderingSystem.deferredPipeline.geometryFb->Diffuse();
			}
			if (ImGui::Selectable("GBuffer: specular"))
			{
				image.texture = data.engine.renderingSystem.deferredPipeline.geometryFb->Specular();
			}
			for (size_t i = 0; i < data.engine.renderingSystem.dirShadowMaps.size(); i++)
			{
				if (ImGui::Selectable(("Dirlight ShadowMap #" + std::to_string(i)).c_str()))
				{
					image.texture = data.engine.renderingSystem.dirShadowMaps[i]->Get();
				}
			}
			for (size_t i = 0; i < data.engine.renderingSystem.pointShadowMaps.size(); i++)
			{
				if (ImGui::Selectable(("Pointlight ShadowMap #" + std::to_string(i)).c_str()))
				{
					image.texture = data.engine.renderingSystem.pointShadowMaps[i]->Get();
				}
			}
			if (ImGui::Selectable("Bool dest"))
			{
				image.texture = data.engine.renderingSystem.bloom.dest->GetAttachments()
					.colors[0].first;
			}
			for (size_t i = 0; i < data.engine.renderingSystem.bloom.fbs.size(); i++)
			{
				if (ImGui::Selectable(("Bloom Texture Helper#" + std::to_string(i)).c_str()))
				{
					image.texture = data.engine.renderingSystem.bloom.fbs[i]->GetAttachments()
						.colors[0].first;
				}
			}
			for (size_t i = 0; i < data.engine.renderingSystem.bloom.fbs2.size(); i++)
			{
				if (ImGui::Selectable(("Bloom2 Texture Helper#" + std::to_string(i)).c_str()))
				{
					image.texture = data.engine.renderingSystem.bloom.fbs2[i]->GetAttachments()
						.colors[0].first;
				}
			}
			
			ImGui::EndPopup();
		}
	}
}

