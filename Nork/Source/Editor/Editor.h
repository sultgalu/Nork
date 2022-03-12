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
		void SetDisplayTexture(std::shared_ptr<Renderer::Texture> tex);
	private:
		EditorData data;
	};
}