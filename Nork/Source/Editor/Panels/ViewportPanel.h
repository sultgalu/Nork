#pragma once

#include "Base/Panel.h"

namespace Nork::Editor
{
	class ViewportPanel : public Panel
	{
	public:
		ViewportPanel();
		~ViewportPanel();

		struct MouseState
		{
			int mousePosX, mousePosY;
			bool isViewportHovered;
			bool isViewportDoubleClicked;
		};
		inline MouseState GetMouseState() { return mouseState; }

	private:
		virtual void DrawContent() override;
	private:
		struct ImageConfig
		{
			glm::vec2 uv_min = glm::vec2(0, 1);
			glm::vec2 uv_max = glm::vec2(1, 0);
			unsigned int texture;
			glm::vec2 resolution = glm::vec2(1920, 1080);
		};

		MouseState mouseState;
		ImageConfig image;
	};
}