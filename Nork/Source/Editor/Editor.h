#pragma once

#include "Core/Engine.h"
#include "EditorData.h"

namespace Nork::Editor
{
	class Editor
	{
	public:
		Editor(Engine& engine);
		void Render();
		void SetDisplayTexture(GLuint tex);
	private:
		EditorData data;
	};
}