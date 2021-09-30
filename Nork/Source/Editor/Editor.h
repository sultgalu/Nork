#pragma once

#include <Platform/Interface/Windows.h>

namespace Nork::Editor
{
	class Editor
	{
	public:
		Editor(Window& win);
		void Render();
		void SetDisplayTexture(GLuint tex);
	};
}