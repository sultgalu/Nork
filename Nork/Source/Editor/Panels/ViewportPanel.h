#pragma once

#include "Base/Panel.h"
#include "Modules/Renderer/Objects/Texture/Texture.h"

namespace Nork::Editor
{
	class ViewportPanel : public Panel
	{
	public:
		ViewportPanel(EditorData& d);
		~ViewportPanel();

		struct MouseState
		{
			int mousePosX = 0, mousePosY = 0;
			bool isViewportHovered = false;
			bool isViewportDoubleClicked = false;
		};
		inline MouseState GetMouseState() { return mouseState; }
		inline void SetTexture(Renderer::Texture& tex) { image.texture = tex; }
	private:
		virtual void Begin() override;
		virtual void DrawContent() override;
	private:
		struct ImageConfig
		{
			glm::vec2 uv_min = glm::vec2(0, 1);
			glm::vec2 uv_max = glm::vec2(1, 0);
			Renderer::Texture texture;
			glm::vec2 resolution = glm::vec2(1920, 1080);
		};

		MouseState mouseState;
		ImageConfig image;
		CameraController camContr;
	};
}