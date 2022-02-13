#include "pch.h"
#include "../ViewportPanel.h"
#include "App/Application.h"

namespace Nork::Editor
{
	static int viewportCounter = 0;
	static Renderer::Texture2D defaultTex;

	ViewportPanel::ViewportPanel(EditorData& d)
		: Nork::Editor::Panel(std::format("Viewport#{}", viewportCounter++), d),
		mouseState(MouseState{}), image(ImageConfig{}),
		camContr(CameraController(events, 
			data.engine.scene.GetMainCamera()))
	{
		uint8_t texData[3] = { 128, 228 , 0 };
		defaultTex.Create().Bind().SetParams().SetData(Renderer::TextureAttributes{ .width = 1, .height = 1, .format = Renderer::TextureFormat::RGB }, texData);
		image.texture = defaultTex;

		static Timer timer;
		events.Subscribe<MouseDownEvent>([&](const MouseDownEvent& e)
			{
				if (e.button != MouseButton::Left)
					return;

				if (data.idQueryMode.test(IdQueryMode::Click))
				{
					data.engine.ReadId(mouseState.mousePosX, mouseState.mousePosY);
				}
				else if (data.idQueryMode.test(IdQueryMode::DoubleClick))
				{
					if (timer.Elapsed() < 500)
					{
						data.engine.ReadId(mouseState.mousePosX, mouseState.mousePosY);
					}
				}
				timer.Restart();
			});
		events.Subscribe<MouseMoveEvent>([&](const MouseMoveEvent& e)
			{
				
				if (data.idQueryMode.test(IdQueryMode::MouseMoveClicked) &&
					Application::Get().inputState.Is<MouseButtonState::Down>(MouseButton::Left))
				{
					data.engine.ReadId(mouseState.mousePosX, mouseState.mousePosY);
				}
				else if (data.idQueryMode.test(IdQueryMode::MouseMoveReleased) &&
					Application::Get().inputState.Is<MouseButtonState::Up>(MouseButton::Left))
				{
					data.engine.ReadId(mouseState.mousePosX, mouseState.mousePosY);
				}
			});
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
		camContr.Update(ImGui::GetIO().DeltaTime * 500);
		ImGui::Begin(this->GetName().data(), 0, ImGuiWindowFlags_NoScrollbar);
	}

	void ViewportPanel::DrawContent()
	{
		data.idQueryMode.reset();

		glm::vec2 texSize(image.resolution.x, image.resolution.y);
		glm::vec2 displaySize = GetDisplaySize(texSize);

		ImGui::Image((ImTextureID)image.texture.GetHandle(), ImVec2(displaySize.x, displaySize.y), ImVec2(image.uv_min.x, image.uv_min.y),
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
		ImGui::Text(("texture ID: " + std::to_string(image.texture.GetHandle())).c_str());

		if (ImGui::BeginPopup("texturesToDisplay"))
		{
			/*if (ImGui::Selectable("DEBUG"))
			{
				image.texture = data.engine.renderingSystem.lightMan.GetDebug();
			}*/
			if (ImGui::Selectable("Default"))
			{
				image.texture = data.engine.renderingSystem.lFb.Color();
			}
			/*if (ImGui::Selectable("Ids"))
			{
				image.texture = data.engine.idMap;
			}*/
			if (ImGui::Selectable("GBuffer: depth"))
			{
				image.texture = data.engine.renderingSystem.gFb.Depth();
			}
			if (ImGui::Selectable("GBuffer: position"))
			{
				image.texture = data.engine.renderingSystem.gFb.Position();
			}
			if (ImGui::Selectable("GBuffer: normal"))
			{
				image.texture = data.engine.renderingSystem.gFb.Normal();
			}
			if (ImGui::Selectable("GBuffer: diffuse"))
			{
				image.texture = data.engine.renderingSystem.gFb.Diffuse();
			}
			if (ImGui::Selectable("GBuffer: specular"))
			{
				image.texture = data.engine.renderingSystem.gFb.Specular();
			}
			for (size_t i = 0; i < data.engine.renderingSystem.dShadowFramebuffers.size(); i++)
			{
				auto& opt = data.engine.renderingSystem.dShadowFramebuffers[i].GetAttachments().depth;
				if (opt.has_value() && ImGui::Selectable(("Dirlight ShadowMap #" + std::to_string(i)).c_str()))
				{
					image.texture = data.engine.renderingSystem.dShadowFramebuffers[i].GetAttachments().depth.value();
				}
			}
			for (size_t i = 0; i < data.engine.renderingSystem.pShadowFramebuffers.size(); i++)
			{
				auto& opt = data.engine.renderingSystem.pShadowFramebuffers[i].GetAttachments().depth;
				if (opt.has_value() && ImGui::Selectable(("Pointlight ShadowMap #" + std::to_string(i)).c_str()))
				{
					image.texture = data.engine.renderingSystem.pShadowFramebuffers[i].GetAttachments().depth.value();
				}
			}
			
			ImGui::EndPopup();
		}
	}
}

